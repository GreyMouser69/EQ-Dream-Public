import assert from 'node:assert/strict';
import {mkdtempSync,mkdirSync,writeFileSync,existsSync,rmSync} from 'node:fs';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {DatabaseSync} from 'node:sqlite';
import {database} from '../eq-test/database.mjs';
import {rewardWorker} from './rewards.mjs';
const root=mkdtempSync(join(tmpdir(),'eq-rewards-')),file=join(root,'cards.sqlite');
const initial=database(file);initial.close();
let worker=rewardWorker(file,join(root,'queue'));const db=new DatabaseSync(file);
const event=(characterId,npcTypeId,kind='named')=>({version:1,characterId,npcTypeId,kind});
const count=()=>db.prepare('SELECT count(*) n FROM packs').get().n;
try{
 assert.equal(worker.grant(event(11,1)),1);
 assert.equal(worker.grant(event(11,1)),0);
 assert.equal(worker.grant(event(11,1,'raid')),0);
 assert.equal(worker.grant(event(11,2,'raid')),5);
 assert.equal(worker.grant(event(11,2)),0);
 assert.equal(worker.grant(event(22,1)),1);
 assert.equal(count(),7);
 for(const e of [event(0,1),event(11,-1),event('11',1),event(11,4,'trash'),{...event(11,4),version:2}])assert.throws(()=>worker.grant(e));
 // Force an insertion failure after the ledger insert: both must roll back.
 db.exec("CREATE TRIGGER fail_reward BEFORE INSERT ON packs WHEN NEW.id LIKE '%:first-kill:99:2' BEGIN SELECT RAISE(ABORT,'test failure'); END");
 assert.throws(()=>worker.grant(event(11,99,'raid')));
 assert.equal(count(),7);assert.equal(db.prepare('SELECT count(*) n FROM tavern_first_kills WHERE npc_type_id=99').get().n,0);
 db.exec('DROP TRIGGER fail_reward');assert.equal(worker.grant(event(11,99,'raid')),5);
 worker.close();worker=rewardWorker(file,join(root,'queue'));assert.equal(worker.grant(event(11,99,'raid')),0);
 // Offline outbox delivery, plus a replay simulating a crash after COMMIT.
 const pending=join(root,'queue','pending'),receipts=join(root,'queue','receipts');
 writeFileSync(join(pending,'33-7.json'),JSON.stringify(event(33,7,'raid')));
 assert.equal(worker.drain(),1);assert.equal(count(),17);assert.ok(existsSync(join(receipts,'33-7.json')));
 writeFileSync(join(pending,'33-7.json'),JSON.stringify(event(33,7,'raid')));
 worker.drain();assert.equal(count(),17);
 // Rewards must not consume the ten welcome packs or prevent starter registration.
 assert.equal(db.prepare("SELECT count(*) n FROM pack_accounts WHERE owner='eq-live:33'").get().n,0);
 console.log('PASS: named 1, raid 5, duplicate suppression, separate characters, validation, atomic rollback, restart and outbox replay.');
}finally{worker.close();db.close();rmSync(root,{recursive:true,force:true});}
