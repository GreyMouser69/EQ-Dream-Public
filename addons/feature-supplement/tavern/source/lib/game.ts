export type Rarity='common'|'uncommon'|'rare'|'mythical';
export const rarityNames:Record<Rarity,string>={common:'Common',uncommon:'Uncommon',rare:'Rare',mythical:'Mythical Rare'};
const rareCards=new Set(['paladin','rally','purify','vampire','terror','deny']);
const mythicalCards=new Set(['griffon','lich']);
const uncommonCards=new Set(['knight','ogre','banner','blade','sun','shade','warden','curse','study','berserker','revenant','feast']);
export const rarityFor=(id:string):Rarity=>mythicalCards.has(id)?'mythical':rareCards.has(id)?'rare':uncommonCards.has(id)?'uncommon':'common';
export type Color='W'|'B'|'G'|'U'|'R';
export const colors:Color[]=['W','B','G','U','R'];
export const divisions:Record<Color,{name:string,clan:string,title:string,theme:string,land:string,art:number}>={
 W:{name:'White',clan:'Warrior',title:'Light of Qeynos',theme:'Protection, soldiers, healing and vigilance',land:'plains',art:7},
 B:{name:'Black',clan:'Necromancer',title:'Shadows of Neriak',theme:'Undead, life drain, death and resurrection',land:'swamp',art:9},
 G:{name:'Green',clan:'Druid',title:'Wilds of Faydark',theme:'Nature, mighty beasts, growth and trample',land:'forest',art:15},
 U:{name:'Blue',clan:'Enchanter',title:'Tides of Erudin',theme:'Arcane knowledge, flying, countermagic and control',land:'island',art:16},
 R:{name:'Red',clan:'Wizard',title:'Flames of Lavastorm',theme:'Fire, direct damage, haste and aggression',land:'mountain',art:17}
};
export const clans=colors.map(color=>divisions[color].clan);
export const clanColor=(clan:string):Color=>colors.find(color=>divisions[color].clan===clan)||'W';
export const emptyPool=():Record<Color,number>=>({W:0,B:0,G:0,U:0,R:0});
export type Card={rarity:Rarity,id:string,name:string,clan:string,cost:number,color:Color,colored:number,attack:number,health:number,type:'Land'|'Creature'|'Instant'|'Sorcery'|'Enchantment'|'Equipment',text:string,effect?:string,keywords?:string[],art:number,encounter?:'Named'|'Raid Boss'};
function c(id:string,name:string,clan:string,cost:number,type:Card['type'],attack:number,health:number,text:string,effect='',keywords:string[]=[],art=2):Card{return {rarity:rarityFor(id),id,name,clan,cost,color:clanColor(clan),colored:type==='Land'?0:1,attack,health,type,text,effect,keywords,art};}
export const cards:Card[]=[
c('plains','Plains of Karana','Warrior',0,'Land',0,0,'Tap: add one white mana.','land',[],0),
c('swamp','Neriak Marsh','Necromancer',0,'Land',0,0,'Tap: add one black mana.','land',[],1),
c('guard','Qeynos Sentinel','Warrior',1,'Creature',1,2,'Vigilance (attacking does not tap this creature).','',['Vigilance']),
c('wolf','Faydark Wolf','Warrior',2,'Creature',3,2,'A swift hunter of the ancient woods.'),
c('knight','Freeport Champion','Warrior',3,'Creature',3,3,'First strike (deals combat damage before other creatures).','',['First strike']),
c('ogre','Oggok Bruiser','Warrior',5,'Creature',5,5,'Trample (excess combat damage reaches the defending hero).','',['Trample']),
c('axe','Crushing Blow','Warrior',2,'Instant',0,0,'Target creature gets +3/+0 until end of turn.','pump',[],4),
c('mend','Mend Wounds','Warrior',1,'Instant',0,0,'You gain 4 life.','heal',[],5),
c('banner','Banner of Qeynos','Warrior',3,'Enchantment',0,0,'Creatures you control get +1/+1.','anthem',[],2),
c('blade','Soulforged Blade','Warrior',2,'Equipment',0,0,'Equipped creature gets +2/+1. Equip 2: attach to your creature at main-phase timing.','equip',[],4),
c('sun','Light of Judgment','Warrior',3,'Sorcery',0,0,'Deal 4 damage to target creature.','smite',[],5),
c('bone','Restless Skeleton','Necromancer',1,'Creature',2,1,'Even death cannot silence its master.','',[],3),
c('shade','Neriak Shade','Necromancer',2,'Creature',2,2,'Flying (can be blocked only by flying creatures).','',['Flying'],3),
c('warden','Crypt Warden','Necromancer',3,'Creature',2,4,'Deathtouch (any combat damage to a creature is lethal).','',['Deathtouch'],3),
c('lich','Ashen Lich','Necromancer',5,'Creature',4,5,'Lifelink (damage dealt also gains you that much life).','',['Lifelink'],3),
c('tap','Siphon Life','Necromancer',3,'Sorcery',0,0,'Target opponent loses 3 life. You gain 3 life.','drain',[],5),
c('fire','Soulfire','Necromancer',2,'Instant',0,0,'Deal 3 damage to target creature or hero.','bolt',[],5),
c('deny','Unravel Soul','Necromancer',2,'Instant',0,0,'Counter target spell on the stack.','counter',[],5),
c('curse','Whispers of Hate','Necromancer',3,'Enchantment',0,0,'Creatures you control get +1/+1.','anthem',[],3),
c('study','Forbidden Knowledge','Necromancer',2,'Sorcery',0,0,'Draw two cards. You lose 2 life.','draw',[],3),
c('griffon','Karana Griffon','Warrior',4,'Creature',3,3,'Flying, vigilance.','',['Flying','Vigilance'],6),
c('paladin','Dawnbringer Paladin','Warrior',3,'Creature',2,3,'Lifelink. When this enters, you gain 3 life.','arrivalHeal',['Lifelink'],7),
c('berserker','Kaladim Berserker','Warrior',3,'Creature',3,2,'Haste (can attack the turn it enters).','',['Haste'],8),
c('rally','Rally the Defenders','Warrior',2,'Instant',0,0,'Your creatures get +1/+1 until end of turn.','rally',[],7),
c('stun','Blinding Radiance','Warrior',1,'Instant',0,0,'Tap target creature. Draw a card. Tapping an attacker does not remove it from combat.','stun',[],7),
c('purify','Purifying Light','Warrior',2,'Sorcery',0,0,'Destroy target enchantment or equipment.','purify',[],7),
c('vampire','Mistmoore Aristocrat','Necromancer',4,'Creature',3,3,'Flying, lifelink.','',['Flying','Lifelink'],9),
c('rats','Plague Rats','Necromancer',2,'Creature',1,3,'Deathtouch.','',['Deathtouch'],10),
c('revenant','Restless Revenant','Necromancer',3,'Creature',3,2,'Haste.','',['Haste'],11),
c('terror','Word of Terror','Necromancer',4,'Instant',0,0,'Destroy target creature.','destroy',[],9),
c('feast','Feast on the Fallen','Necromancer',2,'Sorcery',0,0,'Return the most recently buried creature in your graveyard to your hand. Gain 2 life.','reclaim',[],11),
c('rot','Withering Touch','Necromancer',2,'Instant',0,0,'Target creature gets -2/-2 until end of turn.','shrink',[],10),
 c('forest','Greater Faydark','Druid',0,'Land',0,0,'Tap: add one green mana.','land',[],12),
 c('island','Erudin Coast','Enchanter',0,'Land',0,0,'Tap: add one blue mana.','land',[],13),
 c('mountain','Lavastorm Peaks','Wizard',0,'Land',0,0,'Tap: add one red mana.','land',[],14),
 c('sapling','Faydark Sapling','Druid',1,'Creature',1,2,'A young guardian of the living forest.','',[],15),
 c('panther','Emerald Panther','Druid',2,'Creature',3,2,'The wilds answer to no crown.','',[],15),
 c('bear','Surefall Bear','Druid',3,'Creature',3,4,'Ancient strength beneath a quiet canopy.','',[],15),
 c('growth','Savage Growth','Druid',1,'Instant',0,0,'Target creature gets +3/+0 until end of turn.','pump',[],15),
 c('renewal','Spring Renewal','Druid',1,'Instant',0,0,'You gain 4 life.','heal',[],12),
 c('roots','Reclaiming Roots','Druid',2,'Sorcery',0,0,'Destroy target enchantment or equipment.','purify',[],12),
 c('treant','Faydark Treant','Druid',5,'Creature',5,5,'Trample.','',['Trample'],15),
 c('canopy','Canopy Blessing','Druid',3,'Enchantment',0,0,'Creatures you control get +1/+1.','anthem',[],12),
 c('seed','Seed of Rebirth','Druid',2,'Sorcery',0,0,'Return the most recently buried creature in your graveyard to your hand. Gain 2 life.','reclaim',[],12),
 c('elder','Elder of Tunare','Druid',4,'Creature',4,5,'Vigilance.','',['Vigilance'],15),
 c('stampede','Emerald Stampede','Druid',6,'Creature',7,6,'Trample.','',['Trample'],15),
 c('avatar','Avatar of the Wild','Druid',7,'Creature',8,8,'Trample, vigilance.','',['Trample','Vigilance'],15),
 c('apprentice','Erudin Apprentice','Enchanter',1,'Creature',1,2,'A student of the endless tides.','',[],16),
 c('familiar','Mist Familiar','Enchanter',2,'Creature',1,3,'Flying.','',['Flying'],16),
 c('tideguard','Tidal Guardian','Enchanter',3,'Creature',2,4,'Woven from the depths of the sea.','',[],13),
 c('insight','Arcane Insight','Enchanter',3,'Sorcery',0,0,'Draw two cards.','insight',[],16),
 c('frost','Frostbinding','Enchanter',2,'Instant',0,0,'Tap target creature. Draw a card. Tapping an attacker does not remove it from combat.','stun',[],13),
 c('illusion','Phantasmal Scout','Enchanter',2,'Creature',2,2,'A fleeting echo given form.','',[],16),
 c('dispel','Erudin Dispel','Enchanter',2,'Instant',0,0,'Counter target spell on the stack.','counter',[],16),
 c('sphinx','Sphinx of Erudin','Enchanter',5,'Creature',4,4,'Flying.','',['Flying'],16),
 c('lorekeeper','Tide Lorekeeper','Enchanter',4,'Creature',3,5,'Vigilance.','',['Vigilance'],16),
 c('archmage','Erudin Archmage','Enchanter',4,'Creature',3,4,'Flying.','',['Flying'],16),
 c('deepstudy','Wisdom of the Deep','Enchanter',4,'Instant',0,0,'Draw two cards.','insight',[],13),
 c('leviathan','Prismatic Leviathan','Enchanter',7,'Creature',6,8,'Flying, vigilance.','',['Flying','Vigilance'],13),
 c('spark','Lavastorm Spark','Wizard',1,'Creature',1,1,'Haste.','',['Haste'],17),
 c('raider','Ember Raider','Wizard',2,'Creature',2,1,'Haste.','',['Haste'],17),
 c('cinder','Cinder Hound','Wizard',3,'Creature',4,2,'Born in the heart of the mountain.','',[],14),
 c('firebolt','Lavastorm Bolt','Wizard',2,'Instant',0,0,'Deal 3 damage to target creature or hero.','bolt',[],17),
 c('eruption','Volcanic Eruption','Wizard',3,'Sorcery',0,0,'Deal 4 damage to target creature.','smite',[],14),
 c('fury','Burning Fury','Wizard',1,'Instant',0,0,'Target creature gets +3/+0 until end of turn.','pump',[],17),
 c('flameknight','Flamebound Duelist','Wizard',3,'Creature',3,2,'First strike, haste.','',['First strike','Haste'],17),
 c('warcry','Lavastorm Warcry','Wizard',2,'Instant',0,0,'Your creatures get +1/+1 until end of turn.','rally',[],17),
 c('slag','Slag Giant','Wizard',5,'Creature',6,4,'Trample.','',['Trample'],14),
 c('phoenix','Ashwing Phoenix','Wizard',4,'Creature',3,3,'Flying, haste.','',['Flying','Haste'],17),
 c('inferno','Inferno Titan','Wizard',6,'Creature',6,5,'Haste, trample.','',['Haste','Trample'],17),
 c('dragon','Lord of Lavastorm','Wizard',7,'Creature',7,6,'Flying, haste.','',['Flying','Haste'],17),
];
for(const id of ['treant','canopy','seed','dispel','sphinx','lorekeeper','flameknight','warcry','slag']){const x=cards.find(c=>c.id===id)!;x.rarity='uncommon';}
for(const id of ['elder','stampede','archmage','deepstudy','phoenix','inferno']){const x=cards.find(c=>c.id===id)!;x.rarity='rare';x.colored=2;}
for(const id of ['avatar','leviathan','dragon']){const x=cards.find(c=>c.id===id)!;x.rarity='mythical';x.colored=2;}
// EQ encounter identities; costs and abilities are adaptations for this card game.
function encounter(id:string,name:string,clan:string,cost:number,attack:number,health:number,rarity:Rarity,keywords:string[],art:number,effect='',extra=''){
 const x=c(id,name,clan,cost,'Creature',attack,health,[keywords.join(', '),extra].filter(Boolean).join('. ')+'.',effect,keywords,art);
 x.rarity=rarity;x.colored=rarity==='uncommon'?1:2;x.encounter=rarity==='uncommon'?'Named':'Raid Boss';cards.push(x);
}
encounter('drelzna','Drelzna','Warrior',3,3,3,'uncommon',['First strike'],2);
encounter('dvinn',"D’Vinn",'Necromancer',3,2,3,'uncommon',['Deathtouch'],3);
encounter('grizzleknot','Grizzleknot','Druid',4,4,4,'uncommon',['Trample'],15);
encounter('najena','Najena','Enchanter',3,2,4,'uncommon',['Vigilance'],16);
encounter('fippy','Fippy Darkpaw','Wizard',2,2,2,'uncommon',['Haste'],17);
encounter('avatarwar','The Avatar of War','Warrior',6,6,6,'rare',['First strike','Vigilance'],18);
encounter('mithaniel','Lord Mithaniel Marr','Warrior',7,6,7,'mythical',['Vigilance','Lifelink'],18,'arrivalHeal','When this enters, you gain 3 life');
encounter('venril','Venril Sathir','Necromancer',5,4,5,'rare',['Lifelink','Deathtouch'],19);
encounter('innoruuk','Innoruuk','Necromancer',7,6,6,'mythical',['Flying','Lifelink'],19);
encounter('wuoshi','Wuoshi','Druid',6,6,6,'rare',['Flying','Trample'],20);
encounter('tunare','Tunare','Druid',7,6,8,'mythical',['Vigilance','Trample'],20,'arrivalHeal','When this enters, you gain 3 life');
encounter('phinigel','Phinigel Autropos','Enchanter',5,4,6,'rare',['Vigilance'],21);
encounter('vox','Lady Vox','Enchanter',7,6,7,'mythical',['Flying','Vigilance'],23);
encounter('nagafen','Lord Nagafen','Wizard',6,6,5,'rare',['Flying','Haste'],22);
encounter('fennin','Fennin Ro','Wizard',8,8,7,'mythical',['Haste','Trample'],17);
for(const x of cards){if(x.rarity==='uncommon')uncommonCards.add(x.id);if(x.rarity==='rare')rareCards.add(x.id);if(x.rarity==='mythical')mythicalCards.add(x.id);}
export const card=(id:string)=>{const x=cards.find(c=>c.id===id);if(!x)throw Error('Unknown card.');return x;};
export const isLand=(id:string)=>card(id).type==='Land';
export function starter(clan:string){const sets:Record<string,string[]>={Warrior:['guard','wolf','knight','ogre','axe','mend','banner','blade'],Necromancer:['bone','shade','warden','tap','fire','curse','study','rats'],Druid:['sapling','panther','bear','growth','renewal','roots','treant','canopy'],Enchanter:['apprentice','familiar','tideguard','insight','frost','illusion','dispel','sphinx'],Wizard:['spark','raider','cinder','firebolt','eruption','fury','flameknight','slag']};if(!sets[clan])throw Error('Choose a valid division.');return [...Array(16).fill(divisions[clanColor(clan)].land),...sets[clan].flatMap(id=>[id,id,id])];}
export function expansionDeck(clan:string){if(!['Warrior','Necromancer'].includes(clan))return starter(clan);const ids=clan==='Warrior'?['guard','knight','griffon','paladin','berserker','rally','stun','purify']:['bone','vampire','rats','revenant','terror','feast','rot','deny'];return [...Array(16).fill(clan==='Warrior'?'plains':'swamp'),...ids.flatMap(id=>[id,id,id])];}
export type Unit={id:string,uid:string,tapped:boolean,sick:boolean,damage:number,boost:number,defenseBoost?:number,attached?:string};
export type Player={clan:string,hp:number,pool:Record<Color,number>,hand:string[],deck:string[],board:Unit[],lands:Unit[],grave:string[],landPlayed:boolean};
export type Target={player:number,uid?:string};
export type Spell={uid:string,id:string,owner:number,target?:Target,stackTarget?:string,equip?:string};
export const phases=['main1','attackers','afterAttack','blockers','afterBlock','main2','end'] as const;
export type Game={rules:2,closed?:boolean,players:Player[],turn:number,priority:number,phase:typeof phases[number],round:number,log:string[],winner:number|null,stack:Spell[],passes:number,attackers:string[],blocks:Record<string,string[]>,blocked:string[]};
export type Action={type:string,index?:number,target?:Target,stackTarget?:string,attackers?:string[],blocks?:Record<string,string[]>,uid?:string,order?:string[]};
function shuffle(a:string[]){for(let i=a.length-1;i>0;i--){const j=Math.floor(Math.random()*(i+1));[a[i],a[j]]=[a[j],a[i]];}return a;}
const unit=(id:string):Unit=>({id,uid:crypto.randomUUID(),tapped:false,sick:true,damage:0,boost:0});
function draw(g:Game,i:number){const p=g.players[i],c=p.deck.pop();if(c)p.hand.push(c);else {g.winner=1-i;g.log.push(`${p.clan} cannot draw from an empty library.`);}}
export function stats(g:Game,who:number,u:Unit){const p=g.players[who],c=card(u.id);const anthem=p.board.filter(x=>card(x.id).effect==='anthem').length;const equipment=p.board.filter(x=>x.attached===u.uid).length;return {power:Math.max(0,c.attack+anthem+equipment*2+u.boost),toughness:c.health+anthem+equipment+(u.defenseBoost||0)};}
function check(g:Game){for(let i=0;i<2;i++){const p=g.players[i];const dead=p.board.filter(u=>card(u.id).type==='Creature'&&u.damage>=stats(g,i,u).toughness);p.grave.push(...dead.map(u=>u.id));p.board=p.board.filter(u=>!dead.includes(u));for(const u of p.board)if(u.attached&&!p.board.some(v=>v.uid===u.attached))delete u.attached;}
if(g.players.some(p=>p.hp<=0))g.winner=g.players[0].hp<=0?(g.players[1].hp<=0?2:1):0;g.log=g.log.slice(-24);}
export function newGame(clan:string,deck?:string[]):Game{const players=[clan,clan==='Warrior'?'Necromancer':'Warrior'].map((c,i)=>({clan:c,hp:20,pool:emptyPool(),hand:[],deck:shuffle([...(i===0&&deck?deck:expansionDeck(c))]),board:[],lands:[],grave:[],landPlayed:false} as Player));const g:Game={rules:2,players,turn:0,priority:0,phase:'main1',round:1,log:['Opening hands drawn. Play a land to begin.'],winner:null,stack:[],passes:0,attackers:[],blocks:{},blocked:[]};for(let i=0;i<2;i++)for(let n=0;n<7;n++)draw(g,i);return g;}
export function canPay(p:Player,cost:number,color:Color,colored:number){return (p.pool[color]||0)+p.lands.filter(l=>!l.tapped&&card(l.id).color===color).length>=colored&&colors.reduce((n,col)=>n+(p.pool[col]||0),0)+p.lands.filter(l=>!l.tapped).length>=cost;}
function pay(p:Player,cost:number,color:Color,colored:number){if(!canPay(p,cost,color,colored))throw Error('Not enough untapped lands or mana of the required color.');p.pool={...emptyPool(),...p.pool};while(p.pool[color]<colored){const l=p.lands.find(l=>!l.tapped&&card(l.id).color===color)!;l.tapped=true;p.pool[color]++;}p.pool[color]-=colored;let generic=cost-colored;for(const col of colors){let n=Math.min(generic,p.pool[col]);generic-=n;p.pool[col]-=n;}for(const l of p.lands){if(!generic)break;if(!l.tapped){l.tapped=true;generic--;}}}
const main=(g:Game,i:number)=>g.turn===i&&['main1','main2'].includes(g.phase)&&g.stack.length===0;
function targetUnit(g:Game,t?:Target){return t&&[0,1].includes(t.player)?g.players[t.player].board.find(u=>u.uid===t.uid&&card(u.id).type==='Creature'):undefined;}
function pHasDeadCreature(p:Player){return p.grave.some(id=>card(id).type==='Creature');}
function validTarget(g:Game,s:Spell){const c=card(s.id);if(s.equip)return !!targetUnit(g,s.target)&&s.target?.player===s.owner&&g.players[s.owner].board.some(u=>u.uid===s.equip);if(c.effect==='purify')return !!s.target&&[0,1].includes(s.target.player)&&g.players[s.target.player].board.some(u=>u.uid===s.target!.uid&&['Enchantment','Equipment'].includes(card(u.id).type));if(c.effect==='reclaim')return pHasDeadCreature(g.players[s.owner]);if(['pump','smite','stun','destroy','shrink'].includes(c.effect||''))return !!targetUnit(g,s.target);if(c.effect==='bolt')return !!s.target&&[0,1].includes(s.target.player)&&(!s.target.uid||!!targetUnit(g,s.target));if(c.effect==='counter')return g.stack.some(x=>x.uid===s.stackTarget&&!x.equip);return true;}
function resolve(g:Game){const s=g.stack.pop()!;const p=g.players[s.owner],c=card(s.id);if(!validTarget(g,s)){g.log.push(`${c.name} has no legal target and fizzles.`);if(!s.equip)p.grave.push(s.id);return;}if(s.equip){p.board.find(u=>u.uid===s.equip)!.attached=s.target!.uid;g.log.push('Soulforged Blade is equipped.');return;}
if(['Creature','Enchantment','Equipment'].includes(c.type)){p.board.push(unit(c.id));if(c.effect==='arrivalHeal')p.hp+=3;}else{const t=targetUnit(g,s.target);switch(c.effect){case 'pump':t!.boost+=3;break;case 'rally':p.board.filter(u=>card(u.id).type==='Creature').forEach(u=>{u.boost++;u.defenseBoost=(u.defenseBoost||0)+1;});break;case 'stun':t!.tapped=true;draw(g,s.owner);break;case 'shrink':t!.boost-=2;t!.defenseBoost=(t!.defenseBoost||0)-2;break;case 'destroy':{const other=g.players[s.target!.player];other.board=other.board.filter(u=>u.uid!==t!.uid);other.grave.push(t!.id);break;}case 'purify':{const other=g.players[s.target!.player],ix=other.board.findIndex(u=>u.uid===s.target!.uid);other.grave.push(other.board.splice(ix,1)[0].id);break;}case 'reclaim':{const ix=p.grave.findLastIndex(id=>card(id).type==='Creature');p.hand.push(p.grave.splice(ix,1)[0]);p.hp+=2;break;}case 'heal':p.hp+=4;break;case 'smite':t!.damage+=4;break;case 'bolt':if(t)t.damage+=3;else g.players[s.target!.player].hp-=3;break;case 'drain':g.players[1-s.owner].hp-=3;p.hp+=3;break;case 'counter':{const ix=g.stack.findIndex(x=>x.uid===s.stackTarget);const [other]=g.stack.splice(ix,1);g.players[other.owner].grave.push(other.id);g.log.push(`${card(other.id).name} is countered.`);break;}case 'insight':case 'draw':draw(g,s.owner);if(g.winner===null)draw(g,s.owner);if(c.effect==='draw')p.hp-=2;break;}p.grave.push(c.id);}g.log.push(`${c.name} resolves.`);check(g);}
function combat(g:Game){const a=g.turn,b=1-a;const p=g.players[a],e=g.players[b];const first=new Set([...p.board,...e.board].filter(u=>card(u.id).keywords?.includes('First strike')).map(u=>u.uid));
for(const early of [true,false]){const wounds:Record<string,number>={},lethal=new Set<string>();const hit=(u:Unit,owner:number,v:Unit|undefined,n:number)=>{if(n<=0)return;if(v){wounds[v.uid]=(wounds[v.uid]||0)+n;if(card(u.id).keywords?.includes('Deathtouch'))lethal.add(v.uid);}else g.players[1-owner].hp-=n;if(card(u.id).keywords?.includes('Lifelink'))g.players[owner].hp+=n;};
for(const id of g.attackers){const u=p.board.find(u=>u.uid===id);if(!u)continue;const blockers=(g.blocks[id]||[]).map(id=>e.board.find(u=>u.uid===id)).filter(Boolean) as Unit[];if(first.has(u.uid)===early){let power=stats(g,a,u).power;for(const v of blockers){const n=Math.min(power,card(u.id).keywords?.includes('Deathtouch')?1:Math.max(0,stats(g,b,v).toughness-v.damage));hit(u,a,v,n);power-=n;}if(power>0&&(!g.blocked.includes(id)||card(u.id).keywords?.includes('Trample')))hit(u,a,undefined,power);else if(power>0&&blockers.length)hit(u,a,blockers[blockers.length-1],power);}
for(const v of blockers)if(first.has(v.uid)===early)hit(v,b,u,stats(g,b,v).power);}
for(const who of [a,b])for(const u of g.players[who].board){u.damage+=wounds[u.uid]||0;if(lethal.has(u.uid))u.damage=999;}check(g);if(g.winner!==null)break;}
g.log.push('Combat damage resolves.');g.attackers=[];g.blocks={};g.blocked=[];}
function advance(g:Game){g.players.forEach(p=>p.pool=emptyPool());if(g.phase==='main1')g.phase='attackers';else if(g.phase==='afterAttack')g.phase='blockers';else if(g.phase==='afterBlock'){combat(g);g.phase='main2';}else if(g.phase==='main2')g.phase='end';else if(g.phase==='end'){g.players.forEach(p=>{p.board.forEach(u=>{u.damage=0;u.boost=0;u.defenseBoost=0;});while(p.hand.length>7)p.grave.push(p.hand.pop()!);});g.turn=1-g.turn;if(g.turn===0)g.round++;const p=g.players[g.turn];p.landPlayed=false;[...p.board,...p.lands].forEach(u=>{u.tapped=false;u.sick=false;});g.phase='main1';draw(g,g.turn);g.log.push(`${p.clan} untaps and draws.`);}g.priority=g.turn;}
export function act(g:Game,who:number,a:Action){if(g.rules!==2)throw Error('Start a new duel for the updated rules.');if(g.winner!==null)throw Error('This duel has ended.');const p=g.players[who];if(!p)throw Error('Invalid player.');
if(a.type==='attackers'){if(g.phase!=='attackers'||g.turn!==who)throw Error('Not the declare attackers step.');const ids=a.attackers||[];if(new Set(ids).size!==ids.length)throw Error('Duplicate attacker.');for(const id of ids){const u=p.board.find(u=>u.uid===id);if(!u||card(u.id).type!=='Creature'||u.tapped||(u.sick&&!card(u.id).keywords?.includes('Haste')))throw Error('That creature cannot attack.');}g.attackers=ids;ids.forEach(id=>{const u=p.board.find(u=>u.uid===id)!;if(!card(u.id).keywords?.includes('Vigilance'))u.tapped=true;});g.phase=ids.length?'afterAttack':'main2';g.priority=g.turn;g.passes=0;g.log.push(`${p.clan} declares ${ids.length} attackers.`);return g;}
if(a.type==='blockers'){if(g.phase!=='blockers'||g.turn===who)throw Error('Not the declare blockers step.');const blocks=a.blocks||{},used=new Set<string>();for(const [id,bs] of Object.entries(blocks)){const attacker=g.players[g.turn].board.find(u=>u.uid===id);if(!g.attackers.includes(id)||!attacker||!Array.isArray(bs))throw Error('Invalid attacker.');for(const bid of bs){const u=p.board.find(u=>u.uid===bid);if(!u||card(u.id).type!=='Creature'||u.tapped||used.has(bid))throw Error('Invalid blocker.');if(card(attacker.id).keywords?.includes('Flying')&&!card(u.id).keywords?.includes('Flying'))throw Error('Only flying creatures can block this attacker.');used.add(bid);}}g.blocks=blocks;g.blocked=Object.keys(blocks).filter(k=>blocks[k].length);g.phase='afterBlock';g.priority=g.turn;g.passes=0;g.log.push(`${p.clan} declares ${used.size} blockers.`);return g;}
if(g.phase==='attackers'||g.phase==='blockers')throw Error('Finish declaring combat first.');if(g.priority!==who)throw Error('Wait for priority.');
if(a.type==='order'){const ids=g.blocks[a.uid!]||[];if(g.phase!=='afterBlock'||g.turn!==who||!a.order||a.order.length!==ids.length||new Set(a.order).size!==ids.length||a.order.some(id=>!ids.includes(id)))throw Error('Invalid damage order.');g.blocks[a.uid!]=a.order;return g;}
if(a.type==='tap'){const l=p.lands.find(l=>l.uid===a.uid);if(!l||l.tapped)throw Error('Land is already tapped.');l.tapped=true;p.pool[card(l.id).color]=(p.pool[card(l.id).color]||0)+1;return g;}
if(a.type==='play'){const index=a.index;if(!Number.isInteger(index)||index!<0||!p.hand[index!])throw Error('Choose a card in hand.');const c=card(p.hand[index!]);if(c.type==='Land'){if(!main(g,who)||p.landPlayed)throw Error('Play one land per turn during your main phase.');p.hand.splice(index!,1);p.lands.push(unit(c.id));p.landPlayed=true;g.passes=0;g.log.push(`${p.clan} plays ${c.name}.`);return g;}if(c.type!=='Instant'&&!main(g,who))throw Error('Cast this during your main phase with an empty stack.');const s:Spell={uid:crypto.randomUUID(),id:c.id,owner:who,target:a.target,stackTarget:a.stackTarget};if(!validTarget(g,s))throw Error('Choose a legal target.');pay(p,c.cost,c.color,c.colored);p.hand.splice(index!,1);g.stack.push(s);g.passes=0;g.priority=1-who;g.log.push(`${p.clan} casts ${c.name}.`);
}else if(a.type==='equip'){if(!main(g,who))throw Error('Equip during your main phase with an empty stack.');const u=p.board.find(u=>u.uid===a.uid&&card(u.id).type==='Equipment');if(!u)throw Error('Choose equipment.');const s:Spell={uid:crypto.randomUUID(),id:u.id,owner:who,equip:u.uid,target:a.target};if(!validTarget(g,s))throw Error('Choose your creature.');pay(p,2,'W',0);g.stack.push(s);g.priority=1-who;g.passes=0;
}else if(a.type==='pass'){g.passes++;if(g.passes===2){g.passes=0;if(g.stack.length){resolve(g);g.priority=g.turn;}else advance(g);}else g.priority=1-who;
}else throw Error('Unknown action.');check(g);return g;}
function creatureValue(g:Game,who:number,u:Unit){const c=card(u.id),s=stats(g,who,u);return c.cost+Math.max(0,s.power)*.55+Math.max(0,s.toughness-u.damage)*.35+(c.keywords?.length||0)*.5;}
function positionValue(g:Game,who:number){if(g.winner!==null)return g.winner===2?0:g.winner===who?100000:-100000;return g.players.reduce((n,p,i)=>n+(i===who?1:-1)*(p.hp*.7+p.board.reduce((s,u)=>s+(card(u.id).type==='Creature'?creatureValue(g,i,u):card(u.id).cost),0)),0);}
function combatValue(g:Game,who:number,attackers:string[],blocks:Record<string,string[]>){const copy=structuredClone(g);copy.attackers=attackers;copy.blocks=blocks;copy.blocked=Object.keys(blocks).filter(id=>blocks[id].length);combat(copy);return positionValue(copy,who);}
const canBlock=(a:Unit,b:Unit)=>!b.tapped&&card(b.id).type==='Creature'&&(!card(a.id).keywords?.includes('Flying')||card(b.id).keywords?.includes('Flying'));
export function chooseBlocks(g:Game,defender:number){const e=g.players[1-defender],p=g.players[defender],blocks:Record<string,string[]>={},used=new Set<string>();const attackers=g.attackers.map(id=>e.board.find(u=>u.uid===id)).filter(Boolean) as Unit[];
for(const a of attackers.sort((a,b)=>stats(g,1-defender,b).power-stats(g,1-defender,a).power)){const options=p.board.filter(b=>!used.has(b.uid)&&canBlock(a,b));let best:string[]=[],score=combatValue(g,defender,g.attackers,blocks);const candidates=options.map(u=>[u.uid]);const pairPool=options.slice(0,6);for(let i=0;i<pairPool.length;i++)for(let j=i+1;j<pairPool.length;j++)candidates.push([pairPool[i].uid,pairPool[j].uid]);for(const ids of candidates){const candidate={...blocks,[a.uid]:ids};const value=combatValue(g,defender,g.attackers,candidate);if(value>score+.01){best=ids;score=value;}}if(best.length){blocks[a.uid]=best;best.forEach(id=>used.add(id));}}
return blocks;}
export function chooseAttackers(g:Game,who:number){const p=g.players[who],ready=p.board.filter(u=>card(u.id).type==='Creature'&&!u.tapped&&(!u.sick||card(u.id).keywords?.includes('Haste')));const candidates=ready.map(u=>u.uid);const evaluate=(ids:string[])=>{const copy=structuredClone(g);copy.turn=who;copy.attackers=ids;return combatValue(copy,who,ids,chooseBlocks(copy,1-who));};let chosen=[...candidates],best=evaluate(chosen);for(let pass=0;pass<2;pass++){let changed=false;for(const id of [...chosen].slice(0,12)){const ids=chosen.filter(x=>x!==id),value=evaluate(ids);if(value>best+.01){chosen=ids;best=value;changed=true;}}if(!changed)break;}
return best>evaluate([])+.05?chosen:[];}
function chooseSpell(g:Game,who:number):Action|null{const p=g.players[who],e=g.players[1-who];const candidates:{action:Action,score:number}[]=[];const enemies=e.board.filter(u=>card(u.id).type==='Creature').sort((a,b)=>creatureValue(g,1-who,b)-creatureValue(g,1-who,a));
for(let i=0;i<p.hand.length;i++){const c=card(p.hand[i]);if(c.type==='Land'){if(main(g,who)&&!p.landPlayed)return {type:'play',index:i};continue;}if(!canPay(p,c.cost,c.color,c.colored)||(c.type!=='Instant'&&!main(g,who)))continue;
let target:Target|undefined,stackTarget:string|undefined,score=0;const combatants=[...g.attackers,...Object.values(g.blocks).flat()];
if(c.type==='Creature'){score=4+c.cost+((c.keywords||[]).includes('Haste')?2:0);}
else if(c.effect==='counter'){const top=[...g.stack].reverse().find(s=>s.owner!==who&&!s.equip);if(!top)continue;stackTarget=top.uid;score=8+card(top.id).cost;}
else if(['bolt','smite','destroy','shrink'].includes(c.effect||'')){const amount=c.effect==='smite'?4:c.effect==='shrink'?2:3;const enemy=enemies.find(u=>c.effect==='destroy'||stats(g,1-who,u).toughness-u.damage<=amount);if(c.effect==='bolt'&&e.hp<=3){target={player:1-who};score=1000;}else if(enemy){target={player:1-who,uid:enemy.uid};score=5+creatureValue(g,1-who,enemy);}else continue;}
else if(c.effect==='pump'){if(g.phase!=='afterBlock')continue;let bestGain=.2;for(const u of p.board.filter(u=>combatants.includes(u.uid)&&card(u.id).type==='Creature')){const copy=structuredClone(g);copy.players[who].board.find(v=>v.uid===u.uid)!.boost+=3;const gain=combatValue(copy,who,g.attackers,g.blocks)-combatValue(g,who,g.attackers,g.blocks);if(gain>bestGain){target={player:who,uid:u.uid};bestGain=gain;}}if(!target)continue;score=5+bestGain;}
else if(c.effect==='rally'){if(g.phase!=='afterBlock'||!p.board.some(u=>combatants.includes(u.uid)))continue;const copy=structuredClone(g);copy.players[who].board.filter(u=>card(u.id).type==='Creature').forEach(u=>{u.boost++;u.defenseBoost=(u.defenseBoost||0)+1;});const gain=combatValue(copy,who,g.attackers,g.blocks)-combatValue(g,who,g.attackers,g.blocks);if(gain<=.1)continue;score=5+gain;}
else if(c.effect==='stun'){if(!main(g,who))continue;const u=enemies.find(u=>!u.tapped);if(!u||!p.board.some(u=>card(u.id).type==='Creature'&&!u.tapped&&(!u.sick||card(u.id).keywords?.includes('Haste'))))continue;target={player:1-who,uid:u.uid};score=5;}
else if(c.effect==='purify'){const u=e.board.find(u=>['Equipment','Enchantment'].includes(card(u.id).type));if(!u)continue;target={player:1-who,uid:u.uid};score=7;}
else if(c.effect==='reclaim'){if(!pHasDeadCreature(p))continue;score=6;}
else if(c.effect==='draw'||c.effect==='insight'){if((c.effect==='draw'&&p.hp<=3)||p.deck.length<2)continue;score=p.hand.length<=3?9:3;}
else if(c.effect==='heal'){if(p.hp>12)continue;score=p.hp<=5?12:2;}
else if(c.effect==='drain')score=e.hp<=3?1000:4;
else if(c.effect==='anthem'){score=p.board.filter(u=>card(u.id).type==='Creature').length*2+2;}
else if(c.type==='Equipment'){score=p.board.some(u=>card(u.id).type==='Creature')?3:1;}
else continue;
// Avoid feeding a spell into our own pending spell; wait for its resolution.
if(g.stack.length&&g.stack[g.stack.length-1].owner===who&&c.effect!=='counter')continue;
candidates.push({action:{type:'play',index:i,target,stackTarget},score});}
if(main(g,who)&&canPay(p,2,'W',0)){const eq=p.board.find(u=>card(u.id).type==='Equipment'&&!u.attached),u=p.board.filter(u=>card(u.id).type==='Creature').sort((a,b)=>creatureValue(g,who,b)-creatureValue(g,who,a))[0];if(eq&&u)candidates.push({action:{type:'equip',uid:eq.uid,target:{player:who,uid:u.uid}},score:7});}
return candidates.sort((a,b)=>b.score-a.score)[0]?.action||null;}
export function ai(g:Game){let loops=0;while(g.winner===null&&loops++<80){if(g.phase==='attackers'){if(g.turn!==1)return;act(g,1,{type:'attackers',attackers:chooseAttackers(g,1)});continue;}if(g.phase==='blockers'){if(g.turn===1)return;act(g,1,{type:'blockers',blocks:chooseBlocks(g,1)});continue;}if(g.priority!==1)return;act(g,1,chooseSpell(g,1)||{type:'pass'});}}
