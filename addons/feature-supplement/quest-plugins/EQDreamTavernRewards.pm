package EQDreamTavernRewards;
use strict;
use warnings;
use Fcntl qw(O_RDONLY);
use IO::Handle;
use File::Temp qw(tempfile);
use JSON::PP qw(encode_json);
use EQDreamRelease;
our $ROOT='/var/lib/eqdream-tavern/rewards';

sub killed_merit {
 my ($client,$npc)=@_;
 return 0 unless $client && $npc && $client->IsClient();
 my $kind=EQDreamRelease::role($npc,1);
 return 0 unless $kind eq 'named' || $kind eq 'raid';
 my ($char,$type)=($client->CharacterID(),$npc->GetNPCTypeID());
 return 0 unless $char && $type;
 my $key="$char-$type.json";
 return 0 if -e "$ROOT/receipts/$key" || -e "$ROOT/pending/$key";
 my ($fh,$tmp);
 my $published=0;
 eval {
  ($fh,$tmp)=tempfile('reward-XXXXXXXX',DIR=>$ROOT,UNLINK=>0);
  chmod 0640,$tmp or die "reward permissions: $!";
  print {$fh} encode_json({version=>1,characterId=>0+$char,npcTypeId=>0+$type,kind=>$kind}) or die "reward write: $!";
  $fh->flush() or die "reward flush: $!";
  $fh->sync() or die "reward sync: $!";
  close($fh) or die "reward close: $!";
  # Publish a complete file without overwriting another zone's first-kill event.
  if(link($tmp,"$ROOT/pending/$key")){
   sysopen(my $dir,"$ROOT/pending",O_RDONLY) or die "reward directory open: $!";
   $dir->sync() or die "reward directory sync: $!";
   close $dir;
   $published=1;
  }
  elsif(!-e "$ROOT/pending/$key" && !-e "$ROOT/receipts/$key"){die "reward publish: $!";}
 };
 my $error=$@;
 unlink $tmp if defined($tmp) && -e $tmp;
 if($error){warn "Tavern first-kill reward could not be queued: $error";return 0;}
 if($published){
  my $count=$kind eq 'raid'?5:1;
  $client->Message(15,"[Tavern Duels] First $kind kill! $count pack".($count==1?'':'s')." queued for your collection. Open /tavern to view your packs.");
 }
 return $published;
}
1;
