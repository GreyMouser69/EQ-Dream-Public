import {defineConfig} from 'vite';
import react from '@vitejs/plugin-react';
import {fileURLToPath} from 'node:url';
const path=p=>fileURLToPath(new URL(p,import.meta.url));
export default defineConfig({root:path('./'),base:'/tavern/',publicDir:path('../public'),plugins:[react()],define:{__TAVERN_API_BASE__:JSON.stringify('/tavern/api/game')},resolve:{alias:{'@':path('../')}},css:{postcss:path('../')},build:{outDir:path('../dist-live/web'),emptyOutDir:true}});
