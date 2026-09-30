# Server-only plugin. Tokens are sent privately to the requesting character.
sub TavernAccess {
    my ($who) = @_;
    require JSON::PP;
    require Digest::SHA;
    require Fcntl;
    my $last = $who->GetBucket('tavern-request-time') || 0;
    if (time() - $last < 3) { $who->Message(15, '[Tavern Duels] Please wait a moment, then try again.'); return; }
    $who->SetBucket('tavern-request-time', time(), '10s');
    eval {
        open(my $random, '<:raw', '/dev/urandom') or die 'random unavailable';
        my $bytes = '';
        read($random, $bytes, 32) == 32 or die 'random incomplete';
        close($random);
        my $token = unpack('H*', $bytes);
        my $file = '/var/lib/eqdream-tavern/tickets/' . Digest::SHA::sha256_hex($token) . '.json';
        sysopen(my $out, $file, Fcntl::O_WRONLY() | Fcntl::O_CREAT() | Fcntl::O_EXCL(), 0640) or die 'ticket unavailable';
        print $out JSON::PP::encode_json({ characterId => 0 + $who->CharacterID(), expires => time() + 60 }) or die 'ticket write failed';
        close($out) or die 'ticket save failed';
        $who->Message(15, '[Tavern Access] ' . $token);
    };
    if ($@) { $who->Message(13, '[Tavern Duels] Sign-in is temporarily unavailable. Please try again later.'); }
}
1;
