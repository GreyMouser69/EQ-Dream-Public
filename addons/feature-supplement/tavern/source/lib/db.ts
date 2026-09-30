import {env} from 'cloudflare:workers';
export function db(){const d=(env as unknown as {DB:D1Database}).DB;if(!d)throw Error('Game storage is unavailable. Please try again shortly.');return d;}
