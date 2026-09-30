import {ensureWelcome,claimStarter,openPack,packView} from './packs';
import {cards,clans,starter,newGame,act,ai,Game,isLand} from './game';
type Database=Pick<D1Database,'prepare'|'batch'>;
export function playerView(g:Game,seat:number){return {...g,players:g.players.map((p,i)=>({...p,deck:p.deck.map(()=>'?'),hand:i===seat?p.hand:p.hand.map(()=>'?')}))};}
export async function gameService(d:Database,user:string,b:any){
 if(!b||typeof b!=='object'||typeof b.type!=='string')throw Error('Invalid request.');
 await d.prepare('INSERT OR IGNORE INTO profiles(id,data) VALUES(?,?)').bind(user,JSON.stringify({clan:'Warrior',deck:starter('Warrior'),game:null,catalogVersion:4})).run();
 const row=await d.prepare('SELECT data,version FROM profiles WHERE id=?').bind(user).first<{data:string,version:number}>();
 if(!row)throw Error('Unable to load your profile.');
 const p=JSON.parse(row.data);let changed=false,version=row.version;
 if((p.catalogVersion||0)<4){p.catalogVersion=4;changed=true;}
 await ensureWelcome(d,user);
 let openedPack:any=null;
 const owned=await d.prepare('SELECT card,COUNT(*) AS n FROM inventory WHERE owner=? GROUP BY card').bind(user).all<{card:string,n:number}>();
 const collection=Object.fromEntries(owned.results.map(x=>[x.card,x.n]));
 function validateDeck(clan:string,deck:string[]){
  if(!clans.includes(clan)||!Array.isArray(deck)||deck.length!==40)throw Error('A deck needs exactly 40 cards.');
  const counts:Record<string,number>={};for(const id of deck){const c=cards.find(c=>c.id===id);counts[id]=(counts[id]||0)+1;if(!c||c.type!=='Land'&&(counts[id]>3||counts[id]>(collection[id]||0)))throw Error('Deck contains unavailable cards. Update your deck before playing.');}
 }
 if(b.type==='claim_starter'){await claimStarter(d,user,b.clan);p.clan=b.clan;p.deck=starter(b.clan);changed=true;}
 else if(b.type==='open_pack'){if(typeof b.id!=='string')throw Error('Select a pack.');openedPack=await openPack(d,user,b.id);}
 else if(b.type==='offer'){
  if(!cards.some(c=>c.id===b.wanted&&c.type!=='Land')||!cards.some(c=>c.id===b.offered&&c.type!=='Land'))throw Error('Choose non-land cards for the exchange.');
  const item=await d.prepare("SELECT id FROM inventory WHERE owner=? AND card=? AND id NOT IN (SELECT offered FROM trades WHERE status='open') LIMIT 1").bind(user,b.offered).first<{id:string}>();
  if(!item)throw Error('No unreserved copy available to offer.');
  try{await d.prepare('INSERT INTO trades(id,maker,offered,wanted) VALUES(?,?,?,?)').bind(crypto.randomUUID(),user,item.id,b.wanted).run();}catch{throw Error('That card was just reserved. Refresh before offering another copy.');}
 }else if(b.type==='cancel'){
  const result=await d.prepare("UPDATE trades SET status='cancelled' WHERE id=? AND maker=? AND status='open'").bind(b.id,user).run();
  if(!result.meta.changes)throw Error('This offer is no longer open or is not yours.');
 }else if(b.type==='accept'){
  const t=await d.prepare("SELECT * FROM trades WHERE id=? AND status='open'").bind(b.id).first<{id:string,maker:string,offered:string,wanted:string}>();
  if(!t||t.maker===user)throw Error('Trade is unavailable.');
  const item=await d.prepare("SELECT id FROM inventory WHERE owner=? AND card=? AND id NOT IN (SELECT offered FROM trades WHERE status='open') LIMIT 1").bind(user,t.wanted).first<{id:string}>();
  if(!item)throw Error('You do not have an unreserved copy of the requested card.');
  const result=await d.batch([
   d.prepare("UPDATE inventory SET owner=CASE WHEN id=? THEN ? ELSE ? END WHERE id IN (?,?) AND EXISTS (SELECT 1 FROM trades WHERE id=? AND status='open') AND NOT EXISTS (SELECT 1 FROM trades WHERE offered=? AND status='open') AND EXISTS (SELECT 1 FROM inventory a,inventory b WHERE a.id=? AND a.owner=? AND b.id=? AND b.owner=?)").bind(t.offered,user,t.maker,t.offered,item.id,t.id,item.id,t.offered,t.maker,item.id,user),
   d.prepare("UPDATE trades SET status='complete' WHERE id=? AND status='open' AND EXISTS (SELECT 1 FROM inventory a,inventory b WHERE a.id=? AND a.owner=? AND b.id=? AND b.owner=?)").bind(t.id,t.offered,user,item.id,t.maker)
  ]);
  if(result[0].meta.changes!==2)throw Error('Trade changed; no cards exchanged.');
 }else if(b.type==='deck'){validateDeck(b.clan,b.deck);p.clan=b.clan;p.deck=b.deck;changed=true;}
 else if(b.type==='new'){validateDeck(p.clan,p.deck);if(p.match)throw Error('Leave your multiplayer table before starting practice.');p.game=newGame(p.clan,p.deck);changed=true;}
 else if(b.type==='action'){
  if(b.version!==row.version)throw Error('This duel changed in another window. Reload your saved duel.');
  if(!p.game)throw Error('Start a duel first.');act(p.game,0,b.action);ai(p.game);changed=true;
 }else if(b.type==='host'){
  if(p.match)throw Error('Leave your current table first.');validateDeck(p.clan,p.deck);const id=crypto.randomUUID();
  await d.prepare('INSERT INTO matches(id,host,state) VALUES(?,?,?)').bind(id,user,JSON.stringify(newGame(p.clan,p.deck))).run();p.match=id;changed=true;
 }else if(b.type==='join'){
  if(p.match)throw Error('Leave your current table first.');validateDeck(p.clan,p.deck);
  const m=await d.prepare('SELECT * FROM matches WHERE id=?').bind(String(b.code)).first<any>();
  if(!m||JSON.parse(m.state).rules!==2||JSON.parse(m.state).closed||m.guest||m.host===user)throw Error('This table is unavailable.');
  const game:Game=JSON.parse(m.state);game.players[1]=newGame(p.clan,p.deck).players[0];
  const result=await d.prepare('UPDATE matches SET guest=?,state=?,version=version+1 WHERE id=? AND guest IS NULL AND version=?').bind(user,JSON.stringify(game),m.id,m.version).run();
  if(!result.meta.changes)throw Error('The table changed before you joined.');p.match=m.id;changed=true;
 }else if(b.type==='leave'){
  if(p.match){const m=await d.prepare('SELECT * FROM matches WHERE id=?').bind(p.match).first<any>();
   if(m&&(m.host===user||m.guest===user)){const game:Game=JSON.parse(m.state);if(!game.closed&&game.winner===null){game.closed=true;game.winner=m.guest?(m.host===user?1:0):2;game.log.push(m.guest?'Opponent left the table.':'Table closed.');const result=await d.prepare('UPDATE matches SET state=?,version=version+1 WHERE id=? AND version=?').bind(JSON.stringify(game),m.id,m.version).run();if(!result.meta.changes)throw Error('The table changed. Try leaving again.');}}p.match=null;changed=true;}
 }else if(!['load','pvp'].includes(b.type))throw Error('Unknown request.');
 let match:any=null;
 if(p.match){const m=await d.prepare('SELECT * FROM matches WHERE id=?').bind(p.match).first<any>();
  if(m&&JSON.parse(m.state).rules===2&&(m.host===user||m.guest===user)){
   const seat=m.host===user?0:1,game:Game=JSON.parse(m.state);let matchVersion=m.version;
   if(b.type==='pvp'){
    if(b.matchId!==m.id||b.version!==m.version)throw Error('The match changed. Reload your saved duel.');
    if(!m.guest||game.closed)throw Error('This table is waiting or closed.');act(game,seat,b.action);
    const result=await d.prepare('UPDATE matches SET state=?,version=version+1 WHERE id=? AND version=?').bind(JSON.stringify(game),m.id,m.version).run();
    if(!result.meta.changes)throw Error('The match changed. Reload your saved duel.');matchVersion++;
   }
   match={id:m.id,seat,version:matchVersion,waiting:!m.guest&&!game.closed,game:playerView(game,seat)};
  }else if(b.type==='pvp')throw Error('You are not a player at this table.');
 }else if(b.type==='pvp')throw Error('Join a table first.');
 if(changed){const result=await d.prepare('UPDATE profiles SET data=?,version=version+1 WHERE id=? AND version=?').bind(JSON.stringify(p),user,row.version).run();if(!result.meta.changes)throw Error('Your profile changed in another window. Reload your saved duel.');version++;}
 const counts=await d.prepare('SELECT card,COUNT(*) AS n FROM inventory WHERE owner=? GROUP BY card').bind(user).all<{card:string,n:number}>();
 const offers=await d.prepare("SELECT t.id,t.maker,t.wanted,i.card AS offered FROM trades t JOIN inventory i ON i.id=t.offered AND i.owner=t.maker WHERE t.status='open'").all<any>();
 return {...p,version,packs:await packView(d,user),openedPack,collection:Object.fromEntries(counts.results.map(x=>[x.card,x.n])),offers:offers.results.map(t=>({id:t.id,offered:t.offered,wanted:t.wanted,mine:t.maker===user})),game:p.game?playerView(p.game,0):null,match,user:user.slice(-10)};
}
