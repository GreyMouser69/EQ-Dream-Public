# EQ Dream Anything Slots

## Scope

Two server-authoritative accessory slots, indexed 0 and 1, displayed through the
NMS client window. They are not RoF2 inventory slots and must never be passed to
`InventoryProfile::PutItem` or serialized as native equipment positions.

## Initial item policy

- Equip only from the top-level cursor.
- The destination Anything slot must be empty.
- Withdraw only to an empty cursor.
- Containers and evolving items are rejected in the first release.
- Class, race, required-level, lore, and ownership checks remain enforced.
- Items remain attached to the character on death.

## Transaction contract

Every action includes a login-session nonce, monotonically increasing request
ID, target slot, expected item ID, expected GUID, and expected slot revision.
The server locks the cursor inventory row and Anything row, validates both, and
moves the complete item instance in a single database transaction. The database
commit occurs before the in-memory inventory is changed or success is sent.

Repeated request IDs, stale revisions, mismatched GUIDs, and actions during
trade, loot, bank, merchant, or casting transitions are rejected. After every
success or failure, the server sends an authoritative two-slot refresh.

## Bonus behavior

The two stored `EQ::ItemInstance` objects are included in item-bonus aggregation
through `AddItemBonuses` and, when enabled, `AdditiveWornBonuses`. This covers
base and heroic statistics, AC, HP, mana, endurance, resistances, haste, worn
effects, focus data handled by the item bonus engine, and augments.

Click activation, evolving experience, weapon damage, and weapon procs are not
part of the initial release. Click effects require a dedicated authenticated
request because normal item-click handling assumes a native inventory slot.

## Deployment

Database and code changes are built and tested away from the live server. Live
deployment requires a zone binary update, a client patch, and one scheduled zone
restart. World, login, and existing zones remain untouched during development.
