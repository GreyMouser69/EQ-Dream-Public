import assert from 'node:assert/strict';
import {cards,card,colors,divisions,starter,newGame,emptyPool,canPay,act,ai,rarityFor} from '../lib/game.ts';
const unit=id=>({id,uid:crypto.randomUUID(),tapped:false,sick:false,damage:0,boost:0});
for(const color of colors){
 const d=divisions[color],deck=starter(d.clan);assert.equal(deck.length,40);assert.equal(deck.filter(id=>card(id).type==='Land').length,16);assert.ok(deck.every(id=>card(id).color===color));
 const pool=cards.filter(c=>c.color===color);for(const rarity of ['common','uncommon','rare','mythical'])assert.ok(pool.some(c=>c.rarity===rarity));
 assert.ok(pool.some(c=>c.encounter==='Named'&&c.rarity==='uncommon'));assert.ok(pool.some(c=>c.encounter==='Raid Boss'&&c.rarity==='mythical'));
 const g=newGame(d.clan,deck),p=g.players[0],spell=pool.find(c=>c.type==='Creature'&&c.colored===1);p.hand=[spell.id];p.lands=Array.from({length:spell.cost},()=>unit(d.land));p.pool=emptyPool();
 assert.ok(canPay(p,spell.cost,color,1));act(g,0,{type:'play',index:0});assert.equal(g.stack[0].id,spell.id);act(g,1,{type:'pass'});act(g,0,{type:'pass'});assert.ok(p.board.some(u=>u.id===spell.id));
 const wrong=colors.find(c=>c!==color);p.pool=emptyPool();p.lands=Array.from({length:10},()=>unit(divisions[wrong].land));assert.equal(canPay(p,2,color,1),false);
 p.lands[0]=unit(d.land);assert.equal(canPay(p,3,color,1),true);assert.equal(canPay(p,3,color,2),false);p.pool[color]=1;assert.equal(canPay(p,3,color,2),true);
 // Manual tapping and phase clearing support every color, including old saved pools.
 const h=newGame(d.clan);h.players[0].pool={W:0,B:0};h.players[0].lands=[unit(d.land)];act(h,0,{type:'tap',uid:h.players[0].lands[0].uid});assert.equal(h.players[0].pool[color],1);act(h,0,{type:'pass'});act(h,1,{type:'pass'});assert.deepEqual(h.players[0].pool,emptyPool());
 // AI can cast this division's creatures with its matching lands.
 const bot=newGame(d.clan);bot.players[1]=newGame(d.clan).players[0];bot.players[1].hand=[spell.id];bot.players[1].lands=Array.from({length:spell.cost},()=>unit(d.land));bot.turn=1;bot.priority=1;ai(bot);assert.ok(bot.stack.some(s=>s.id===spell.id));
}
const g=newGame('Enchanter');g.players[0].hand=['insight'];g.players[0].lands=Array.from({length:3},()=>unit('island'));const hp=g.players[0].hp;act(g,0,{type:'play',index:0});act(g,1,{type:'pass'});act(g,0,{type:'pass'});assert.equal(g.players[0].hand.length,2);assert.equal(g.players[0].hp,hp,'Blue draw does not charge black life-loss');
for(const c of cards)assert.equal(c.rarity,rarityFor(c.id));
assert.equal(new Set(cards.map(c=>c.id)).size,cards.length);
console.log('Five-color tests passed: starters, rarity pools, named/raid tiers, casting, mixed generic payment, double colored costs, legacy pools, phase clearing, blue draw and AI.');
