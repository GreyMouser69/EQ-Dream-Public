import {DatabaseSync} from 'node:sqlite';
import {mkdirSync,opendirSync,readFileSync,renameSync} from 'node:fs';
import {resolve} from 'node:path';

// Only the server's private filesystem outbox can issue rewards. No HTTP route.
export function rewardWorker(filename,root){
 const pending=resolve(root,'pending'),receipts=resolve(root,'receipts');
 for(const dir of [pending,receipts])mkdirSync(dir,{recursive:true});
 const db=new DatabaseSync(filename);
 db.exec(`PRAGMA busy_timeout=1000; PRAGMA synchronous=FULL;
 CREATE TABLE IF NOT EXISTS tavern_first_kills(
 character_id INTEGER NOT NULL,npc_type_id INTEGER NOT NULL,
 kind TEXT NOT NULL CHECK(kind IN ('named','raid')),
 packs INTEGER NOT NULL CHECK(packs IN (1,5)),awarded_at INTEGER NOT NULL,
 PRIMARY KEY(character_id,npc_type_id));`);
 function grant(event){
  if(!event||event.version!==1||!['named','raid'].includes(event.kind)||
   ![event.characterId,event.npcTypeId].every(n=>Number.isSafeInteger(n)&&n>0&&n<=4294967295))throw Error('Invalid reward event');
  const count=event.kind==='raid'?5:1,owner='eq-live:'+event.characterId;
  db.exec('BEGIN IMMEDIATE');
  try{
   const inserted=db.prepare('INSERT OR IGNORE INTO tavern_first_kills VALUES(?,?,?,?,?)').run(event.characterId,event.npcTypeId,event.kind,count,Date.now()).changes;
   if(inserted){
    const add=db.prepare("INSERT INTO packs(id,owner,tier) VALUES(?,?,'standard')");
    for(let i=0;i<count;i++)add.run(`${owner}:first-kill:${event.npcTypeId}:${i}`,owner);
   }
   db.exec('COMMIT');return inserted?count:0;
  }catch(e){db.exec('ROLLBACK');throw e;}
 }
 function drain(){
  const dir=opendirSync(pending);let processed=0;
  try{let entry;while(processed<128&&(entry=dir.readSync())){
   if(!entry.isFile()||!/^\d+-\d+\.json$/.test(entry.name))continue;
   processed++;
   try{
    const source=resolve(pending,entry.name),text=readFileSync(source,'utf8');
    if(text.length>1024)throw Error('Oversized reward');
    const event=JSON.parse(text);
    if(entry.name!==`${event.characterId}-${event.npcTypeId}.json`)throw Error('Reward identity mismatch');
    grant(event);
    // A crash after COMMIT leaves a replayable event; the ledger prevents a second grant.
    renameSync(source,resolve(receipts,entry.name));
   }catch(e){console.error('Tavern reward delivery pending:',entry.name,e.message);}
  }}finally{dir.closeSync();}
  return processed;
 }
 return {grant,drain,close(){db.close()}};
}
