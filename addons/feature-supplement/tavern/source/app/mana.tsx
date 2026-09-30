'use client';
import {Sun,Skull,Leaf,Droplet,Flame} from 'lucide-react';
import {Card,Color,colors,divisions} from '@/lib/game';
const icons={W:Sun,B:Skull,G:Leaf,U:Droplet,R:Flame};
export function ManaSymbol({color}:{color:Color}){const Icon=icons[color];return <span className={'manaSymbol mana-'+color} title={divisions[color].name+' mana'} aria-label={divisions[color].name+' mana'}><Icon aria-hidden="true"/></span>}
export function ManaCost({card:c}:{card:Card}){const generic=c.cost-c.colored;return <span className="manaCost" aria-label={c.type==='Land'?divisions[c.color].name+' land':`${generic?generic+' generic plus ':''}${c.colored} ${divisions[c.color].name} mana`}>
{c.type==='Land'?<ManaSymbol color={c.color}/>:<>{generic>0&&<span className="manaGeneric">{generic}</span>}{Array.from({length:c.colored},(_,i)=><ManaSymbol key={i} color={c.color}/>)}</>}</span>}
export function ManaPool({pool}:{pool:Partial<Record<Color,number>>}){return <span className="manaPool">{colors.map(color=><span key={color}><ManaSymbol color={color}/><b>{pool[color]||0}</b></span>)}</span>}
