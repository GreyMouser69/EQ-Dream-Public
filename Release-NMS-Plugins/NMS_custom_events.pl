sub CustomEventSayEntry {
    return 0;
}

sub CustomEventNPCDeathEntry {
    return 0;
}

sub CustomEventHandinEntry {
    return 0;
}

sub CustomEventNPCSpawnEntry {
    return 0;
}

sub CustomEventExpGainEntry {
    return 0;
}

sub CustomEventAAExpGainEntry {
    return 0;
}

sub CustomEventItemEquipEntry {
    return 0;
}

sub CustomEventItemUnequipEntry {
    return 0;
}

sub CustomEventDestroyEntry {
    return 0;
}

sub CustomEventItemClickCastEntry {
    return 0;
}

sub UpdateEoMAward {
    my $client = shift || plugin::val('$client');
    return 0 unless $client && $client->IsClient();

    # EQ Dream starter grant: award each character once, on its first login.
    # The character bucket prevents the balance from refilling after it is spent.
    my $starter_bucket = 'eqdream.eom_starter_10000';
    return 0 if $client->GetBucket($starter_bucket);

    my $starter_amount = 10000;
    my $current_amount = plugin::GetEOM($client);
    if ($current_amount < $starter_amount) {
        plugin::LootEOM($client, $starter_amount - $current_amount);
    }

    $client->SetBucket($starter_bucket, '1');
    return 1;
}

sub DoEventRewards {
    return 0;
}

1;
