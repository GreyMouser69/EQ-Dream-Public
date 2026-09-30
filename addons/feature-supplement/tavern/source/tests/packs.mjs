import assert from 'node:assert/strict';
import {DatabaseSync} from 'node:sqlite';
import {readFileSync} from 'node:fs';
import ts from 'typescript';
const source=readFileSync(new URL('../lib/game-service.ts',import.meta.url),'utf8');
const packSource=readFileSync(new URL('../lib/packs.ts',import.meta.url),'utf8');
const packJs=ts.transpileModule(packSource,{compilerOptions:{module:ts.ModuleKind.ESNext,target:ts.ScriptTarget.ES2022}}).outputText.replace("from './game'",`from '${new URL('../lib/game.ts',import.meta.url).href}'`);
const packUrl='data:text/javascript;base64,'+Buffer.from(packJs).toString('base64');
const js=ts.transpileModule(source,{compilerOptions:{module:ts.ModuleKind.ESNext,target:ts.ScriptTarget.ES2022}}).outputText.replace("from './game'",`from '${new URL('../lib/game.ts',import.meta.url).href}'`);
const {gameService}=await import('data:text/javascript;base64,'+Buffer.from(js.replace("from './packs'",`from '${packUrl}'`)).toString('base64'));
const sqlite=new DatabaseSync(':memory:');sqlite.exec(readFileSync(new URL('../drizzle/0000_minor_molecule_man.sql',import.meta.url),'utf8'));
sqlite.exec(readFileSync(new URL('../drizzle/0001_glamorous_tattoo.sql',import.meta.url),'utf8'));
let failBatch=false;
function statement(sql,args=[]){const run=()=>{const r=sqlite.prepare(sql).run(...args);return {success:true,meta:{changes:Number(r.changes)}};};return {bind(...values){return statement(sql,values)},async first(){return sqlite.prepare(sql).get(...args)||null},async all(){return {results:sqlite.prepare(sql).all(...args)}},async run(){return run()},runSync:run};}
const db={prepare:statement,async batch(items){sqlite.exec('BEGIN');try{const out=items.map((s,i)=>{if(failBatch&&i===1)throw Error('Injected second-statement failure');return s.runSync();});sqlite.exec('COMMIT');return out;}catch(e){sqlite.exec('ROLLBACK');throw e;}}};
const call=(u,b)=>gameService(db,u,b);
const {rollPack,packRules}=await import(packUrl);
const {card,starter}=await import('../lib/game.ts');
let a=await call('newcomer',{type:'load'});
assert.equal(a.packs.unopened.length,10);assert.ok(a.packs.unopened.every(p=>p.tier==='standard'));assert.equal(Object.keys(a.collection).length,0,'New accounts should not receive the whole collection');
await Promise.all([call('newcomer',{type:'load'}),call('newcomer',{type:'load'})]);assert.equal((await call('newcomer',{type:'load'})).packs.unopened.length,10);
await call('other',{type:'load'});const id=a.packs.unopened[0].id;
await assert.rejects(()=>call('other',{type:'open_pack',id}),/belong/);
a=await call('newcomer',{type:'claim_starter',clan:'Necromancer'});assert.equal(a.deck.length,40);assert.equal(a.deck.filter(id=>card(id).type==='Land').length,16);assert.equal(Object.values(a.collection).reduce((n,c)=>n+c,0),24);assert.ok(a.deck.every(id=>['common','uncommon'].includes(card(id).rarity)));assert.equal(a.packs.starterClan,'Necromancer');
await assert.rejects(()=>call('newcomer',{type:'claim_starter',clan:'Warrior'}),/already/);assert.equal(Object.values((await call('newcomer',{type:'load'})).collection).reduce((n,c)=>n+c,0),24);
const before=(await call('newcomer',{type:'load'})).collection;failBatch=true;await assert.rejects(()=>call('newcomer',{type:'open_pack',id}),/Injected/);failBatch=false;
a=await call('newcomer',{type:'load'});assert.equal(a.packs.unopened.length,10);assert.deepEqual(a.collection,before);
const pair=await Promise.all([call('newcomer',{type:'open_pack',id}),call('newcomer',{type:'open_pack',id})]);assert.deepEqual(pair[0].openedPack.cards,pair[1].openedPack.cards);assert.equal(pair[0].openedPack.cards.length,10);
a=await call('newcomer',{type:'load'});assert.equal(a.packs.unopened.length,9);assert.equal(a.packs.history.length,1);assert.equal(Object.values(a.collection).reduce((n,c)=>n+c,0),34);
const again=await call('newcomer',{type:'open_pack',id});assert.deepEqual(again.openedPack.cards,pair[0].openedPack.cards);assert.equal(Object.values(again.collection).reduce((n,c)=>n+c,0),34);
assert.ok(a.packs.unopened.every(p=>!('contents' in p)&&!('cards' in p)),'Sealed contents must not be exposed');
for(const p of a.packs.unopened)await call('newcomer',{type:'open_pack',id:p.id});a=await call('newcomer',{type:'load'});assert.equal(a.packs.unopened.length,0);assert.equal(a.packs.history.length,10);assert.equal(Object.values(a.collection).reduce((n,c)=>n+c,0),124);
await call('newcomer',{type:'load'});assert.equal((await call('newcomer',{type:'load'})).packs.unopened.length,0,'Reload must not replenish the free packs');
// Exhaust all 50 equally likely final-slot outcomes: exactly one mythical.
const frequencies={rare:0,mythical:0};
for(let roll=0;roll<50;roll++){let first=true;const ids=rollPack(n=>{if(first){first=false;assert.equal(n,50);return roll;}return 0;});assert.equal(ids.length,10);assert.ok(ids.slice(0,7).every(id=>card(id).rarity==='common'));assert.ok(ids.slice(7,9).every(id=>card(id).rarity==='uncommon'));assert.ok(ids.every(id=>card(id).type!=='Land'));frequencies[card(ids[9]).rarity]++;}
assert.deepEqual(frequencies,{rare:49,mythical:1});
// Previously issued tiers must all open under the same new rules.
await call('legacy',{type:'load'});
for(const tier of ['common','uncommon','rare','mythical']){
 const packId='legacy:'+tier;sqlite.prepare('INSERT INTO packs(id,owner,tier) VALUES(?,?,?)').run(packId,'legacy',tier);
 const opened=(await call('legacy',{type:'open_pack',id:packId})).openedPack;
 assert.equal(opened.cards.length,10);assert.equal(opened.cards.filter(id=>card(id).rarity==='common').length,7);assert.equal(opened.cards.filter(id=>card(id).rarity==='uncommon').length,2);
}
// A concurrent starter claim chooses exactly one class and grants exactly 24 spells.
await call('race',{type:'load'});const starters=await Promise.allSettled([call('race',{type:'claim_starter',clan:'Warrior'}),call('race',{type:'claim_starter',clan:'Necromancer'})]);assert.equal(starters.filter(r=>r.status==='fulfilled').length,1);const winner=await call('race',{type:'load'});assert.equal(Object.values(winner.collection).reduce((n,c)=>n+c,0),24);assert.deepEqual(winner.deck,starter(winner.packs.starterClan));
for(const clan of ['Druid','Enchanter','Wizard']){
 const user='division:'+clan;await call(user,{type:'load'});const d=await call(user,{type:'claim_starter',clan});assert.equal(d.deck.length,40);assert.equal(Object.values(d.collection).reduce((n,c)=>n+c,0),24);
 // A genuinely owned off-color card can be used with lands of either color.
 sqlite.prepare('INSERT INTO inventory(id,owner,card) VALUES(?,?,?)').run(user+':mixed',user,'guard');const deck=[...d.deck];deck[0]='plains';deck[16]='guard';const saved=await call(user,{type:'deck',clan,deck});assert.deepEqual(saved.deck,deck);assert.ok((await call(user,{type:'new'})).game);
}
console.log('Packs passed: welcome/starter grants, five-division ownership and mixed decks, slot odds, rollback, concurrent opening, retry recovery and history.');sqlite.close();
