# Test-only Luclin chase forms. Run before the normal click spell/charge gate.
# Reuse the established transform_item helper; do not alter other eras.
sub LuclinChaseIsForm {
    my ($id) = @_;
    return $id >= 900701 && $id <= 900705;
}
sub LuclinChaseCycle {
    my ($client, $item_id, $slot_id) = @_;
    my %forms = (
        900704 => 900701, # Moonwarden Mace -> Crescent Blade
        900701 => 900702, # Crescent Blade -> Eclipse Greatsword
        900702 => 900705, # Eclipse Greatsword -> Astral Staff
        900705 => 900703, # Astral Staff -> Voidfang
        900703 => 900704, # Voidfang -> Moonwarden Mace
    );
    return 0 unless exists $forms{$item_id};
    # Never delete a stale/mismatched source slot.
    if ($client->GetItemIDAt($slot_id) != $item_id) {
        $client->Message(13, "Weapon switching stopped: the clicked item has moved.");
        return 1;
    }
    # Existing copies may retain zero charges from their original no-click form.
    # Repair only this verified instance, before the C++ zero-charge gate.
    # Successful transformation saves a replacement with one non-consuming charge.
    my $instance = $client->GetItemAt($slot_id);
    if ($instance && $instance->GetCharges() <= 0) {
        $instance->SetCharges(1);
    }
    # The established helper puts the replacement and detached augments on cursor.
    # Active server uses RoF2 slots:33 is cursor;30 is an ordinary inventory slot.
    if ($slot_id == 33 || $client->GetItemIDAt(33) > 0) {
        $client->Message(13, "Put away your cursor items before changing weapon form.");
        return 1;
    }
    # SummonItem would reject duplicate lore AFTER the old helper deletes source.
    # HasItem covers bags, bank and cursor; conservatively block any duplicate.
    if ($client->GetInventory()->HasItem($forms{$item_id}) != -1) {
        $client->Message(13, "You already own the next weapon form. Store it on another character before switching.");
        return 1;
    }
    my $key = 'luclin-chase-cycle-ready';
    if (($client->GetBucket($key) || 0) > time()) {
        $client->Message(13, "Wait six seconds between weapon form changes.");
        return 1;
    }
    my $result = plugin::transform_item($client, $item_id, $slot_id, \%forms, 1);
    if ($result) {
        $client->SetBucket($key, time() + 6, '6s');
        $client->Message(15, "Weapon form changed. Your new weapon and any socketed augments are on your cursor; equip and re-socket them.");
    }
    return 1;
}
1;
