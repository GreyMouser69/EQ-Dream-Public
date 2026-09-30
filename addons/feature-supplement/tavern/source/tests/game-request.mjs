import assert from 'node:assert/strict';
import {gameRequest,GameRequestError} from '../lib/game-request.ts';
let calls=0;const ok={deck:[],collection:{},game:{round:5}};
const result=await gameRequest({type:'load'},async()=>new Response(JSON.stringify(ok),{headers:{'Content-Type':'application/json'}}));assert.equal(result.game.round,5);
await assert.rejects(()=>gameRequest({type:'action'},async()=>{calls++;return new Response('<html>Sign in</html>');}),e=>e instanceof GameRequestError&&e.kind==='connection');assert.equal(calls,1);
await assert.rejects(()=>gameRequest({type:'load'},async()=>new Response('{}',{status:401})),e=>e.kind==='session');
await assert.rejects(()=>gameRequest({type:'action'},async()=>{throw new DOMException('The string did not match the expected pattern.','SyntaxError')}),e=>e.kind==='connection'&&!e.message.includes('pattern'));
await assert.rejects(()=>gameRequest({type:'action'},async()=>new Response(JSON.stringify({error:'Wait for priority.'}),{status:400})),e=>e.kind==='rules'&&e.message==='Wait for priority.');
console.log('Response recovery tests passed: valid, HTML, session expiry, Safari error, rule rejection, no mutation retry.');
