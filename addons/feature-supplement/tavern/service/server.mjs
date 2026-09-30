import http from 'node:http';
import {randomBytes,createHash} from 'node:crypto';
import {readFileSync,mkdirSync,statSync,existsSync,renameSync,unlinkSync,readdirSync} from 'node:fs';
import {resolve,sep,extname} from 'node:path';
import {fileURLToPath} from 'node:url';
import {database} from './database.mjs';
import {gameService} from './rules/game-service.mjs';
import {rewardWorker} from './rewards.mjs';
const hash=s=>createHash('sha256').update(s).digest('hex');
export async function startServer({port=17867,state,tickets,rewards,origin='https://eq.example.org',secure=true}){
 mkdirSync(state,{recursive:true});mkdirSync(tickets,{recursive:true});
 const db=database(resolve(state,'cards.sqlite'));
 await db.prepare('CREATE TABLE IF NOT EXISTS tavern_sessions(id TEXT PRIMARY KEY, player TEXT NOT NULL, expires INTEGER NOT NULL)').run();
 const reward=rewards?rewardWorker(resolve(state,'cards.sqlite'),rewards):null;
 const deliver=()=>{try{reward?.drain()}catch(e){console.error('Tavern reward worker:',e.message)}};
 deliver();const rewardTimer=reward?setInterval(deliver,2000):null;
 let serial=Promise.resolve();const web=resolve(fileURLToPath(new URL('./web',import.meta.url)));
 const limits=new Map();
 const server=http.createServer(async(req,res)=>{
  res.setHeader('Cache-Control','no-store');res.setHeader('Referrer-Policy','no-referrer');res.setHeader('X-Content-Type-Options','nosniff');
  const reply=(s,b)=>{res.writeHead(s,{'Content-Type':'application/json'});res.end(JSON.stringify(b))};
  try{
   const url=new URL(req.url,origin);
   if(req.headers.host!==new URL(origin).host)return reply(403,{error:'Invalid host.'});
   if(url.pathname==='/health'&&req.method==='GET')return reply(200,{service:'eq-tavern-live',ready:true});
   if(url.pathname==='/bootstrap'&&req.method==='GET'){
    const token=url.searchParams.get('ticket')||'';
    if(!/^[a-f0-9]{64}$/.test(token))return reply(401,{error:'Open the game using /tavern in EQ Dream.'});
    const path=resolve(tickets,hash(token)+'.json'),claimed=path+'.claimed';
    let ticket;
    try{renameSync(path,claimed);try{ticket=JSON.parse(readFileSync(claimed,'utf8'))}finally{unlinkSync(claimed)}}catch{return reply(401,{error:'That sign-in expired. Use /tavern again.'})}
    if(!Number.isSafeInteger(ticket.characterId)||ticket.characterId<1||!Number.isSafeInteger(ticket.expires)||ticket.expires<Date.now()/1000)return reply(401,{error:'That sign-in expired. Use /tavern again.'});
    const session=randomBytes(32).toString('hex');
    await db.prepare('INSERT INTO tavern_sessions VALUES(?,?,?)').bind(hash(session),'eq-live:'+ticket.characterId,Date.now()+12*3600000).run();
    res.writeHead(303,{'Location':'/tavern/','Set-Cookie':`tavern_session=${session}; Path=/tavern/; HttpOnly; SameSite=Strict; Max-Age=43200${secure?'; Secure':''}`});return res.end();
   }
   const raw=(req.headers.cookie||'').split(';').map(s=>s.trim()).find(s=>s.startsWith('tavern_session='))?.slice(15)||'';
   const session=/^[a-f0-9]{64}$/.test(raw)?await db.prepare('SELECT player,expires FROM tavern_sessions WHERE id=?').bind(hash(raw)).first():null;
   if(!session||session.expires<Date.now())return reply(401,{error:'Open Tavern Duels from your EQ character with /tavern.'});
   if(url.pathname==='/api/game'&&req.method==='POST'){
    if(req.headers.origin!==origin||!/^application\/json(?:;|$)/i.test(req.headers['content-type']||''))return reply(403,{error:'Invalid origin.'});
    const now=Date.now(),limit=limits.get(session.player)||{time:now,count:0};
    if(now-limit.time>10000){limit.time=now;limit.count=0}limits.set(session.player,limit);
    if(++limit.count>60)return reply(429,{error:'Please wait a moment before trying again.'});
    let body='';for await(const c of req){body+=c;if(Buffer.byteLength(body)>32768)return reply(413,{error:'Request too large.'})}
    let data;try{data=JSON.parse(body)}catch{return reply(400,{error:'Invalid JSON.'})}
    const operation=serial.then(()=>gameService(db,session.player,data));serial=operation.catch(()=>{});
    try{return reply(200,await operation)}catch(e){return reply(400,{error:e.message})}
   }
   if(req.method!=='GET')return reply(405,{error:'Method unavailable.'});
   const file=resolve(web,'.'+decodeURIComponent(url.pathname==='/'?'/index.html':url.pathname));
   if(!file.startsWith(web+sep)||!existsSync(file)||!statSync(file).isFile())return reply(404,{error:'Not found.'});
   res.setHeader('Content-Security-Policy',"default-src 'self'; script-src 'self'; style-src 'self' 'unsafe-inline'; img-src 'self' data:; font-src 'self' data:; connect-src 'self'; object-src 'none'; base-uri 'none'; frame-ancestors 'none'");
   res.setHeader('Content-Type',({'.html':'text/html; charset=utf-8','.js':'text/javascript','.css':'text/css','.png':'image/png','.svg':'image/svg+xml'})[extname(file)]||'application/octet-stream');res.end(readFileSync(file));
  }catch{if(!res.headersSent)reply(500,{error:'Tavern Duels is temporarily unavailable.'});else res.end()}
 });
 await new Promise((ok,no)=>{server.once('error',no);server.listen(port,'127.0.0.1',ok)});
 const cleanup=setInterval(async()=>{
  try{await db.prepare('DELETE FROM tavern_sessions WHERE expires<?').bind(Date.now()).run();
   for(const name of readdirSync(tickets).filter(n=>/^[a-f0-9]{64}\.json(?:\.claimed)?$/.test(n))){const file=resolve(tickets,name);try{if(Date.now()-statSync(file).mtimeMs>180000)unlinkSync(file)}catch{}}
   for(const [id,l]of limits)if(Date.now()-l.time>60000)limits.delete(id);
  }catch{}
 },60000);
 return {port:server.address().port,async close(){clearInterval(cleanup);clearInterval(rewardTimer);await new Promise(ok=>server.close(ok));await serial;reward?.close();db.close()}};
}
if(process.argv[1]&&resolve(process.argv[1])===fileURLToPath(import.meta.url)){
 const app=await startServer(JSON.parse(readFileSync(process.argv[2],'utf8')));console.log('Tavern Duels live service ready.');
 for(const signal of ['SIGINT','SIGTERM'])process.on(signal,async()=>{await app.close();process.exit(0)});
}
