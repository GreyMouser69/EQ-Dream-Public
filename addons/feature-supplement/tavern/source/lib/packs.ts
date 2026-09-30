import {cards,clans,starter,Rarity} from './game';
export const packRules={description:'10 cards: 7 common, 2 uncommon and 1 rare. The rare slot has a 1 in 50 (2%) chance to become a mythical rare.'};
const welcomePacks=Array.from({length:10},(_,i)=>i);
function randomIndex(n:number){const max=Math.floor(4294967296/n)*n;const a=new Uint32Array(1);do{crypto.getRandomValues(a);}while(a[0]>=max);return a[0]%n;}
export function rollPack(pick:(n:number)=>number=randomIndex){
 const final:Rarity=pick(50)===0?'mythical':'rare';
 const slots:Rarity[]=[...Array<Rarity>(7).fill('common'),'uncommon','uncommon',final];
 return slots.map(r=>{const pool=cards.filter(c=>c.type!=='Land'&&c.rarity===r);if(!pool.length)throw Error('Pack pool unavailable.');return pool[pick(pool.length)].id;});
}
type Database=Pick<D1Database,'prepare'|'batch'>;
export async function ensureWelcome(d:Database,user:string){
 if(await d.prepare('SELECT owner FROM pack_accounts WHERE owner=?').bind(user).first())return;
 // Deterministic grant IDs make reconnects and concurrent first visits safe.
 await d.batch([d.prepare('INSERT OR IGNORE INTO pack_accounts(owner) VALUES(?)').bind(user),...welcomePacks.map(i=>d.prepare('INSERT OR IGNORE INTO packs(id,owner,tier) VALUES(?,?,?)').bind(user+':welcome-pack:'+i,user,'standard'))]);
}
export async function claimStarter(d:Database,user:string,clan:string){if(!clans.includes(clan))throw Error('Choose one of the five divisions.');
 const nonce=crypto.randomUUID();
 // Starter card IDs are unique to this claim attempt. The account update and
 // inventory writes run in one transaction; only the winning clan is granted.
 const result=await d.batch([d.prepare('UPDATE pack_accounts SET starter_clan=? WHERE owner=? AND starter_clan IS NULL').bind(clan+'|'+nonce,user),...starter(clan).filter(id=>cards.find(c=>c.id===id)!.type!=='Land').map((id,i)=>d.prepare('INSERT INTO inventory(id,owner,card) SELECT ?,?,? WHERE EXISTS (SELECT 1 FROM pack_accounts WHERE owner=? AND starter_clan=?)').bind(user+':starter:'+i,user,id,user,clan+'|'+nonce)),d.prepare('UPDATE pack_accounts SET starter_clan=? WHERE owner=? AND starter_clan=?').bind(clan,user,clan+'|'+nonce)]);
 if(!result[0].meta.changes)throw Error('Your free starter deck has already been claimed.');
}
export async function openPack(d:Database,user:string,id:string){
 const pack=await d.prepare('SELECT * FROM packs WHERE id=? AND owner=?').bind(id,user).first<any>();if(!pack)throw Error('This pack does not belong to you.');
 if(pack.contents)return {id:pack.id,tier:'standard',cards:JSON.parse(pack.contents),openedAt:pack.opened_at};
 const contents=rollPack(),key=crypto.randomUUID(),now=Date.now();
 await d.batch([d.prepare('UPDATE packs SET contents=?,opening_key=?,opened_at=? WHERE id=? AND owner=? AND contents IS NULL').bind(JSON.stringify(contents),key,now,id,user),...contents.map((card,i)=>d.prepare('INSERT INTO inventory(id,owner,card) SELECT ?,?,? WHERE EXISTS (SELECT 1 FROM packs WHERE id=? AND owner=? AND opening_key=?)').bind(id+':card:'+i,user,card,id,user,key))]);
 const opened=await d.prepare('SELECT id,tier,contents,opened_at FROM packs WHERE id=? AND owner=?').bind(id,user).first<any>();
 if(!opened?.contents)throw Error('Pack opening did not finish. You can safely retry this pack.');
 return {id:opened.id,tier:'standard',cards:JSON.parse(opened.contents),openedAt:opened.opened_at};
}
export async function packView(d:Database,user:string){const account=await d.prepare('SELECT starter_clan FROM pack_accounts WHERE owner=?').bind(user).first<any>();const result=await d.prepare('SELECT id,tier,contents,opened_at FROM packs WHERE owner=? ORDER BY opened_at DESC,id').bind(user).all<any>();return {starterClan:account?.starter_clan||null,unopened:result.results.filter(p=>!p.contents).map(p=>({id:p.id,tier:'standard'})),history:result.results.filter(p=>p.contents).map(p=>({id:p.id,tier:'standard',cards:JSON.parse(p.contents),openedAt:p.opened_at}))};}
