declare const __TAVERN_API_BASE__: string;
export class GameRequestError extends Error {
  constructor(message:string, public kind:'session'|'connection'|'rules'){super(message);}
}
// Mutations are sent once. Only read-back is retried, because a lost response
// does not prove that a move/trade failed to commit on the server.
export async function gameRequest(body:unknown, fetcher:typeof fetch=fetch):Promise<any>{
 const controller=new AbortController();const timeout=setTimeout(()=>controller.abort(),15000);
 try{
  const endpoint=typeof __TAVERN_API_BASE__==='string'?__TAVERN_API_BASE__:'/api/game';
  const response=await fetcher(endpoint,{method:'POST',credentials:'same-origin',cache:'no-store',headers:{'Content-Type':'application/json','Accept':'application/json'},body:JSON.stringify(body),signal:controller.signal});
  if(response.status===401||response.redirected&&/signin|login|oauth/.test(response.url))throw new GameRequestError('Your sign-in needs to be renewed. Your saved duel will remain available.','session');
  const raw=await response.text();
  let data:any;try{data=JSON.parse(raw);}catch{throw new GameRequestError('The connection returned an incomplete response. Reload your saved duel to continue.','connection');}
  if(!response.ok)throw new GameRequestError(typeof data?.error==='string'?data.error:'The game service is temporarily unavailable.',response.status>=500?'connection':'rules');
  if(!data||typeof data!=='object'||!Array.isArray(data.deck)||!data.collection)throw new GameRequestError('The game response was incomplete. Reload your saved duel.','connection');
  return data;
 }catch(e){if(e instanceof GameRequestError)throw e;throw new GameRequestError('The connection was interrupted. Reload your saved duel to continue.','connection');}
 finally{clearTimeout(timeout);}
}
