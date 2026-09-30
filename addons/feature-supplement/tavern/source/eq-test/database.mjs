import {DatabaseSync} from 'node:sqlite';
import {readFileSync} from 'node:fs';
export function database(filename){
 const sqlite=new DatabaseSync(filename);
 sqlite.exec('PRAGMA journal_mode=WAL; PRAGMA foreign_keys=ON; CREATE TABLE IF NOT EXISTS eq_migrations(name TEXT PRIMARY KEY)');
 for(const name of ['0000_minor_molecule_man.sql','0001_glamorous_tattoo.sql']){
  if(sqlite.prepare('SELECT name FROM eq_migrations WHERE name=?').get(name))continue;
  sqlite.exec('BEGIN');
  try{sqlite.exec(readFileSync(new URL('../drizzle/'+name,import.meta.url),'utf8'));sqlite.prepare('INSERT INTO eq_migrations VALUES(?)').run(name);sqlite.exec('COMMIT');}
  catch(e){sqlite.exec('ROLLBACK');sqlite.close();throw e;}
 }
 function statement(sql,args=[]){
  const run=()=>{const r=sqlite.prepare(sql).run(...args);return {success:true,meta:{changes:Number(r.changes)}};};
  return {bind(...v){return statement(sql,v)},async first(){return sqlite.prepare(sql).get(...args)||null},async all(){return {results:sqlite.prepare(sql).all(...args)}},async run(){return run()},runSync:run};
 }
 return {prepare:statement,async batch(items){sqlite.exec('BEGIN');try{const out=items.map(s=>s.runSync());sqlite.exec('COMMIT');return out;}catch(e){sqlite.exec('ROLLBACK');throw e;}},close(){sqlite.close()}};
}
