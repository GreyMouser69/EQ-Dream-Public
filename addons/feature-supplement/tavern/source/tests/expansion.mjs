import assert from 'node:assert/strict';
import {newGame,act,ai,card,stats,chooseAttackers,chooseBlocks,expansionDeck} from '../lib/game.ts';
const u=(id,extra={})=>({id,uid:crypto.randomUUID(),tapped:false,sick:false,damage:0,boost:0,...extra});
const fresh=()=>newGame('Warrior');const resolve=g=>{act(g,g.priority,{type:'pass'});act(g,g.priority,{type:'pass'});};
function cast(g,id,target){g.players[0].hand=[id];g.players[0].lands=Array.from({length:6},()=>u(card(id).color==='W'?'plains':'swamp'));act(g,0,{type:'play',index:0,target});resolve(g);}
let g=fresh();g.phase='attackers';const haste=u('berserker',{sick:true});g.players[0].board=[haste];act(g,0,{type:'attackers',attackers:[haste.uid]});assert.equal(haste.tapped,true);
g=fresh();cast(g,'paladin');assert.equal(g.players[0].hp,23);assert.equal(g.players[0].board[0].id,'paladin');
g=fresh();const guard=u('guard');g.players[0].board=[guard];cast(g,'rally');assert.deepEqual(stats(g,0,guard),{power:2,toughness:3});g.phase='end';resolve(g);assert.deepEqual(stats(g,0,guard),{power:1,toughness:2});
g=fresh();const bone=u('bone');g.players[1].board=[bone];cast(g,'rot',{player:1,uid:bone.uid});assert.equal(g.players[1].board.length,0);
g=fresh();const banner=u('banner'),wolf=u('wolf',{damage:2});g.players[1].board=[banner,wolf];cast(g,'purify',{player:1,uid:banner.uid});assert.equal(g.players[1].board.length,0);assert.deepEqual(g.players[1].grave,['banner','wolf']);
g=fresh();g.players[0].grave=['bone','fire','vampire'];cast(g,'feast');assert.ok(g.players[0].hand.includes('vampire'));assert.equal(g.players[0].hp,22);assert.ok(g.players[0].grave.includes('bone'));
g=fresh();const lich=u('lich');g.players[1].board=[lich];cast(g,'terror',{player:1,uid:lich.uid});assert.equal(g.players[1].board.length,0);
g=fresh();const enemy=u('wolf');g.players[1].board=[enemy];const hand=g.players[0].hand.length;cast(g,'stun',{player:1,uid:enemy.uid});assert.ok(enemy.tapped);assert.equal(g.players[0].hand.length,1);
// Smarter combat: avoid losing a skeleton to a free block; take free damage.
g=fresh();g.turn=1;g.phase='attackers';g.players[1].board=[u('bone')];g.players[0].board=[u('ogre')];assert.deepEqual(chooseAttackers(g,1),[]);g.players[0].board=[];assert.equal(chooseAttackers(g,1).length,1);
// Defend lethal even at the cost of a creature.
g=fresh();const ogre=u('ogre'),warden=u('warden');g.players[0].board=[ogre];g.players[1].board=[warden];g.attackers=[ogre.uid];g.players[1].hp=2;const blocks=chooseBlocks(g,1);assert.ok(blocks[ogre.uid]?.includes(warden.uid));
// Use two profitable blockers together rather than making a losing single block.
g=fresh();const attacker=u('ogre'),one=u('knight'),two=u('knight');g.players[0].board=[attacker];g.players[1].board=[one,two];g.attackers=[attacker.uid];assert.equal(chooseBlocks(g,1)[attacker.uid]?.length,2);
assert.equal(expansionDeck('Warrior').length,40);assert.equal(expansionDeck('Necromancer').length,40);
console.log('Expansion effects, cleanup, haste, AI profitable attacks, lethal defense and double blocking passed.');
