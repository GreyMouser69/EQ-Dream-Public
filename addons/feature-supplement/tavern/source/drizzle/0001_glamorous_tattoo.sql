CREATE TABLE `pack_accounts` (
	`owner` text PRIMARY KEY NOT NULL,
	`starter_clan` text
);
--> statement-breakpoint
CREATE TABLE `packs` (
	`id` text PRIMARY KEY NOT NULL,
	`owner` text NOT NULL,
	`tier` text NOT NULL,
	`contents` text,
	`opening_key` text,
	`opened_at` integer
);
--> statement-breakpoint
CREATE INDEX `packs_owner` ON `packs` (`owner`);