import assert from 'node:assert/strict';
import {mkdtempSync,mkdirSync,writeFileSync,rmSync} from 'node:fs';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {createHash,randomBytes} from 'node:crypto';
import {startServer} from '../dist-live/server.mjs';
const state=mkdtempSync(join(tmpdir(),'tavern-live-')),tickets=join(state,'tickets');mkdirSync(tickets);
const origin='https://eq.example.org';
let service=await startServer({port:0,state,tickets,origin});
async function request(path,{cookie='',body,extra={}}={}){
 return fetch(`http://127.0.0.1:${service.port}${path}`,{method:body?'POST':'GET',redirect:'manual',headers:{Host:new URL(origin).host,Origin:origin,Cookie:cookie,'Content-Type':'application/json',...extra},...(body?{body:JSON.stringify(body)}:{})});
}
// Node's fetch ignores the Host override; use a local HTTP proxy with the real Host.
import http from 'node:http';
request=async function(path,{cookie='',body,extra={}}={}){
 return new Promise((ok,no)=>{const req=http.request({hostname:'127.0.0.1',port:service.port,path,method:body?'POST':'GET',headers:{Host:new URL(origin).host,Origin:origin,Cookie:cookie,'Content-Type':'application/json',...extra}},res=>{let text='';res.on('data',c=>text+=c);res.on('end',()=>ok({status:res.statusCode,headers:res.headers,json:()=>JSON.parse(text),text}))});req.on('error',no);req.end(body?JSON.stringify(body):undefined)});
};
function ticket(id,expires=Date.now()/1000+60){const token=randomBytes(32).toString('hex');writeFileSync(join(tickets,createHash('sha256').update(token).digest('hex')+'.json'),JSON.stringify({characterId:id,expires:Math.floor(expires)}));return token;}
async function login(id){const token=ticket(id);const result=await request('/bootstrap?ticket='+token);assert.equal(result.status,303);assert.match(result.headers['set-cookie'][0],/Secure/);assert.equal((await request('/bootstrap?ticket='+token)).status,401);return result.headers['set-cookie'][0].split(';')[0];}
try{
 assert.equal((await request('/api/game',{body:{type:'load'},extra:{'oai-authenticated-user-id':'admin'}})).status,401);
 assert.equal((await request('/bootstrap?ticket='+ticket(1,0))).status,401);
 assert.equal((await request('/bootstrap?ticket='+ticket('Alice'))).status,401);
 const alice=await login(11),bob=await login(22);
 assert.equal((await request('/api/game',{cookie:alice,body:{type:'load'},extra:{Origin:'https://evil.example'}})).status,403);
 assert.equal((await request('/health',{extra:{Host:'evil.example'}})).status,403);
 assert.equal((await request('/../server.mjs',{cookie:alice})).status,404);
 assert.equal((await request('/',{cookie:alice})).status,200);
 const call=async(cookie,body)=>{const r=await request('/api/game',{cookie,body});assert.equal(r.status,200,r.text);return r.json()};
 await call(alice,{type:'claim_starter',clan:'Warrior'});await call(bob,{type:'claim_starter',clan:'Necromancer'});
 let a=await call(alice,{type:'host'});const b=await call(bob,{type:'join',code:a.match.id});assert.equal(b.match.seat,1);
 a=await call(alice,{type:'load'});assert.ok(a.match.game.players[1].hand.every(c=>c==='?'));
 await call(bob,{type:'leave'});await call(alice,{type:'leave'});
 a=await call(alice,{type:'offer',offered:'guard',wanted:'bone'});const offer=a.offers.find(o=>o.mine);await call(bob,{type:'accept',id:offer.id});
 const saved=await call(alice,{type:'load'});await service.close();service=await startServer({port:0,state,tickets,origin});
 assert.deepEqual((await call(alice,{type:'load'})).collection,saved.collection,'session and cards survive service restart');
 console.log('Live service passed: server-issued one-use sign-in, expiry, character validation, host/origin checks, hidden hands, shared multiplayer, trading and persistent sessions/cards.');
}finally{await service.close();rmSync(state,{recursive:true,force:true});}
