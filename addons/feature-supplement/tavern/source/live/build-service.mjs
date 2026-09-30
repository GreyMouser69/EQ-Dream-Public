import ts from 'typescript';
import {mkdirSync,readFileSync,writeFileSync,copyFileSync} from 'node:fs';
const out=new URL('../dist-live/',import.meta.url);
mkdirSync(new URL('rules/',out),{recursive:true});
mkdirSync(new URL('drizzle/',out),{recursive:true});
for(const name of ['game','packs','game-service']){
 let js=ts.transpileModule(readFileSync(new URL('../lib/'+name+'.ts',import.meta.url),'utf8'),{compilerOptions:{module:ts.ModuleKind.ESNext,target:ts.ScriptTarget.ES2022}}).outputText;
 js=js.replace(/from '\.\/(game|packs)'/g,"from './$1.mjs'");
 writeFileSync(new URL('rules/'+name+'.mjs',out),js);
}
copyFileSync(new URL('./server.mjs',import.meta.url),new URL('server.mjs',out));
copyFileSync(new URL('./rewards.mjs',import.meta.url),new URL('rewards.mjs',out));
writeFileSync(new URL('database.mjs',out),readFileSync(new URL('../eq-test/database.mjs',import.meta.url),'utf8').replace("'../drizzle/'","'./drizzle/'"));
for(const name of ['0000_minor_molecule_man.sql','0001_glamorous_tattoo.sql'])copyFileSync(new URL('../drizzle/'+name,import.meta.url),new URL('drizzle/'+name,out));
