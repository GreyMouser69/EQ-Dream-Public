package EQDreamRelease;
use strict;
use warnings;
use EQDreamPersonalLoot;
use EQDreamEncounter;
use EQDreamAbilityPack;
use EQDreamSunLoot;
use EQDreamVeliousLoot;
our %classic=map {$_=>1} (32040,73057,64001,72003,76007,76011,72090);
# The Hole uses alternate Master Yael ID 2000382. Chase eligibility only.
our %classic_chase=map {$_=>1} (keys %classic,2000382);
our %named_exception=map {$_=>1} (63003,63010,63017,63052,63086);
our %trigger=(marked_detonation=>'eqd_enc_repeat',volatile_ground=>'eqd_vg_repeat',
 deaths_command=>'eqd_dc_repeat',gravity_well=>'eqd_gw_repeat',repulsion_wave=>'eqd_rw_repeat',
 arcane_lockout=>'eqd_al_cast',blood_price=>'eqd_bp_repeat',predators_focus=>'eqd_pf_repeat',unstable_summons=>'eqd_us_repeat');
sub epic_nagafen {
 my ($ctx)=@_;
 return 0 unless $ctx->{npc} && ($ctx->{instanceversion}//0)==30;
 return ($ctx->{npc}->GetNPCTypeID()==32040 && ($ctx->{zonesn}//'') eq 'soldungb')
     || ($ctx->{npc}->GetNPCTypeID()==73057 && ($ctx->{zonesn}//'') eq 'permafrost');
}
sub cycle_seconds { return epic_nagafen($_[0]) ? 10 : 40; }
sub stop_repeats {quest::stoptimer($_) for values %trigger;}
sub cycle {
 my ($ctx,$assigned)=@_;my $n=$ctx->{npc};return unless EQDreamEncounter::active($n);
 if ($n->GetNPCTypeID()==73057 && epic_nagafen($ctx)) {
  return if (0+($n->GetEntityVariable('vox_pause_until')||0))>time();
  $n->SetEntityVariable('vox_last_raid_cycle',time());
 }
 my @list=grep {exists $trigger{$_}} split /,/,$assigned;return unless @list;
 my $i=(0+($n->GetEntityVariable('eqd_cycle_index') || 0)) % @list;
 $n->SetEntityVariable('eqd_cycle_index',''.(($i+1)%@list));
 $n->SetEntityVariable('eqd_last_ability',$list[$i]);
 call_ability($list[$i],'EVENT_TIMER',{%$ctx,timer=>$trigger{$list[$i]}});
 stop_repeats();
}
sub role {
 my ($n,$include_dead)=@_;
 return 'normal' unless $n && ($include_dead || $n->GetHP()>0) && !$n->GetOwnerID() && !$n->IsCharmed()
   && !$n->GetSwarmOwner() && $n->GetClass()>=1 && $n->GetClass()<=16;
 return 'normal' if $n->GetNPCTypeID()>=900200 && $n->GetNPCTypeID()<=900299;
 return 'raid' if $n->IsRaidTarget() || $classic{$n->GetNPCTypeID()};
 return 'named' if $n->IsRareSpawn();
 # Older content often leaves rare_spawn unset, including classic Unrest named.
 my $name=$n->GetName();
 return 'named' if $named_exception{$n->GetNPCTypeID()} || $name =~ /^[#!]/
   || ($name =~ /^[A-Z]/ && $name !~ /^(?:A|An)[_ ]/);
 return 'normal';
}
sub call_ability {
 my ($ability,$event,$context)=@_;
 return unless $event eq 'EVENT_DEATH' || EQDreamEncounter::raid_boss($context->{npc});
 return unless grep {$_ eq $ability} @EQDreamEncounter::ABILITIES;
 my $pkg="EQDreamAbilityPack::$ability";
 no strict 'refs';
 my $handler=$pkg->can($event);return unless $handler;
 local ${"${pkg}::npc"}=$context->{npc};
 local ${"${pkg}::entity_list"}=$context->{entity_list};
 local ${"${pkg}::combat_state"}=$context->{combat_state};
 local ${"${pkg}::timer"}=$context->{timer};
 local ${"${pkg}::entity_id"}=$context->{entity_id};
 local ${"${pkg}::damage"}=$context->{damage};
 local ${"${pkg}::is_damage_shield"}=$context->{is_damage_shield};
 local ${"${pkg}::is_buff_tic"}=$context->{is_buff_tic};
 $handler->();
}
# Native FD-immunity snapshot used only to undo pre-update Death's Command on named mobs.
our %legacy_native_fd = (21165=>1,21166=>1,21167=>1,21168=>1,21169=>1,22186=>1,186209=>1,186210=>1,202382=>1,296080=>1,296081=>1,304015=>1,304016=>1,304018=>1,365009=>1,365014=>1,365019=>1,365020=>1,365023=>1,365035=>1,365038=>1,365058=>1,365083=>1,365085=>1,365087=>1,365143=>1,365173=>1,365174=>1,365175=>1,999000=>1,999001=>1,999002=>1,999003=>1,999004=>1,999005=>1,999006=>1,999007=>1,999008=>1,999009=>1,999010=>1,999011=>1,999012=>1,999013=>1,999014=>1,999017=>1,999019=>1,999022=>1,999024=>1,999027=>1,999029=>1,999032=>1,999101=>1,999102=>1,999103=>1,999104=>1,999105=>1,999106=>1,999107=>1,999108=>1,999109=>1,999110=>1,999111=>1,999112=>1,999113=>1,999114=>1,999115=>1,999116=>1,999117=>1,999118=>1,999119=>1,999120=>1,999121=>1,999122=>1,999123=>1,999124=>1,999125=>1,999126=>1,999127=>1,999128=>1,999129=>1,999130=>1,999131=>1,999132=>1,999133=>1,999134=>1,999135=>1,999136=>1,999137=>1,999138=>1,999139=>1,999140=>1,999201=>1,999202=>1,999203=>1,999204=>1,999205=>1,999206=>1,999207=>1,999208=>1,999209=>1,999210=>1,999211=>1,999212=>1,2000274=>1,2000276=>1,2000672=>1,2000748=>1);
sub disable_named_abilities {
 my ($ctx,$assigned)=@_;my $n=$ctx->{npc};
 return unless $assigned || $n->GetEntityVariable('eqd_initialized');
 quest::stoptimer('eqd_cycle');
 call_ability($_,'EVENT_DEATH',$ctx) for grep {length} split /,/,$assigned;
 stop_repeats();
 if (grep {$_ eq 'deaths_command'} split /,/,$assigned) {
   $n->SetSpecialAbility(27,$legacy_native_fd{$n->GetNPCTypeID()} || 0);
 }
 $n->SetEntityVariable($_,'') for qw(eqd_assigned eqd_initialized eqd_role eqd_cycle_index eqd_last_ability);
}
# Reviewed Planes of Power raid NPCs; passive avoidance bypass, once per NPC.
our %pop_strikethrough_boss = map {$_=>1} (200007,200055,204065,205091,206046,206074,207001,208074,209026,212014,212023,212025,212026,212033,212055,212063,214026,214052,214083,214108,214109,214110,214111,214113,215056,216048,216094,217054,217076,220006,220015,221008,222008,222013,222014,222025,223075,223076,223077,223078,223142,223164,223166,223167,223168,223201);
sub apply_pop_strikethrough {
 my ($n,$event)=@_;
 return unless $n && $pop_strikethrough_boss{$n->GetNPCTypeID()};
 return if $n->GetOwnerID() || $n->IsCharmed() || $n->GetSwarmOwner();
 return if $event ne 'EVENT_SPAWN' && $n->GetEntityVariable('eqd_pop_strikethrough_25');
 $n->RemoveAISpellEffect(196);
 $n->RemoveAISpellEffect(292);
 $n->AddAISpellEffect(196,25,0,0);
 $n->SetEntityVariable('eqd_pop_strikethrough_25','1');
}
sub dispatch {
 my ($event,$ctx)=@_;
 return 0 unless ($ENV{EQDREAM_ENCOUNTER_MODULE}//'') eq '1';
 my $n=$ctx->{npc};return 0 unless $n;
 my $assigned=$n->GetEntityVariable('eqd_assigned');
 # Keep role() unchanged for first-kill packs; only raids receive encounter abilities.
 if (!EQDreamEncounter::raid_boss($n)) {disable_named_abilities($ctx,$assigned);return 0;}
 apply_pop_strikethrough($n,$event);
 if (!$n->GetEntityVariable('eqd_initialized')) {
   my $role='raid'; # The explicit registry gate above is authoritative.
   my $list=epic_nagafen($ctx) ? [@EQDreamEncounter::ABILITIES] : EQDreamEncounter::assign($role,sub{rand()});
   $assigned=join(',',@$list);
   $n->SetEntityVariable('eqd_assigned',$assigned);
   $n->SetEntityVariable('eqd_initialized','1');
   $n->SetEntityVariable('eqd_role',$role);
   { local $|=1; print 'EQDreamRelease assigned npc='.$n->GetNPCTypeID().' entity='.$n->GetID().' role='.$role.' abilities='.$assigned."\n"; }
   call_ability($_,'EVENT_SPAWN',$ctx) for @$list;
 }
 # Reconcile existing Epic spawns without resetting HP or encounter phases.
 if (epic_nagafen($ctx)) {
   my %had=map {$_=>1} split /,/,($assigned||'');
   my @missing=grep {!$had{$_}} @EQDreamEncounter::ABILITIES;
   if (@missing) {
     $assigned=join(',',@EQDreamEncounter::ABILITIES);
     $n->SetEntityVariable('eqd_assigned',$assigned);
     call_ability($_,'EVENT_SPAWN',$ctx) for @missing;
   }
 }
 return 0 if $event eq 'EVENT_SPAWN';
 return 0 if $event eq 'EVENT_TIMER' && ($ctx->{timer}//'') !~ /^eqd_/;
 if ($event eq 'EVENT_COMBAT') {
   quest::stoptimer('eqd_cycle');
   call_ability($_,$event,$ctx) for grep {length} split /,/,$assigned;
   stop_repeats();
   if (($ctx->{combat_state}//0)==1) {
     $n->SetEntityVariable('eqd_cycle_index','0');
     if (EQDreamEncounter::active($n)) {cycle($ctx,$assigned);quest::settimer('eqd_cycle',cycle_seconds($ctx));}
     else {quest::settimer('eqd_cycle',1);} # Native hate-list transition may finish after EVENT_COMBAT.
   }
   return 0;
 }
 if ($event eq 'EVENT_TIMER' && $ctx->{timer} eq 'eqd_cycle') {
   unless (EQDreamEncounter::active($n)) {quest::stoptimer('eqd_cycle');return 0;}
   cycle($ctx,$assigned);quest::settimer('eqd_cycle',cycle_seconds($ctx));return 0;
 }
 if ($event eq 'EVENT_TIMER' && grep {$_ eq $ctx->{timer}} values %trigger) {quest::stoptimer($ctx->{timer});return 0;}
 quest::stoptimer('eqd_cycle') if $event eq 'EVENT_DEATH';
 call_ability($_,$event,$ctx) for grep {length} split /,/,$assigned;
 stop_repeats();
 return 0; # Never override existing quest damage/death results.
}
sub chase {
 my ($n,$entities,$corpse_id,$roll,$choice)=@_;
 my $last=0;
 for my $corpse (EQDreamPersonalLoot::targets($entities,$corpse_id)) {
  my $item=_chase_one($n,$entities,$corpse->GetID(),$roll,$choice);
  $last=$item if $item;
 }
 return $last;
}
sub _chase_one {
 my ($n,$entities,$corpse_id,$roll,$choice)=@_;
 return 0 unless ($ENV{EQDREAM_CHASE_LOOT_MODULE}//'') eq '1';
 return 0 unless $n && !$n->GetOwnerID() && !$n->IsCharmed();
 my $velious=EQDreamVeliousLoot::eligible($n->GetNPCTypeID());
 my $sun=($ENV{EQDREAM_KUNARK_SUN_LOOT}//'') eq '1' && EQDreamSunLoot::eligible($n->GetNPCTypeID());
 return 0 unless $velious || $sun || $classic_chase{$n->GetNPCTypeID()};
 return 0 if ($velious || $sun) && $n->GetSwarmOwner();
 return 0 unless $corpse_id;
 my $corpse=$entities->GetCorpseByID($corpse_id);return 0 unless $corpse;
 return 0 if $corpse->GetEntityVariable('eqd_chase_rolled');
 $corpse->SetEntityVariable('eqd_chase_rolled','1');
 $roll=rand() unless defined $roll;$choice=int(rand($velious?4:3)) unless defined $choice;
 return 0 unless $roll>=0 && $roll<0.05 && $choice>=0 && $choice<=($velious?3:2) && int($choice)==$choice;
 my $item=$velious ? EQDreamVeliousLoot::choice($roll,$choice) : $sun ? EQDreamSunLoot::choice($roll,$choice) : (900100,900101,900102)[$choice];
 $corpse->AddItem($item,1);return $item;
}
1;
