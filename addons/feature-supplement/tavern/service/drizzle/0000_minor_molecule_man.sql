CREATE TABLE `inventory` (
	`id` text PRIMARY KEY NOT NULL,
	`owner` text NOT NULL,
	`card` text NOT NULL
);
--> statement-breakpoint
CREATE INDEX `inventory_owner` ON `inventory` (`owner`);--> statement-breakpoint
CREATE TABLE `matches` (
	`id` text PRIMARY KEY NOT NULL,
	`host` text NOT NULL,
	`guest` text,
	`state` text NOT NULL,
	`version` integer DEFAULT 0 NOT NULL
);
--> statement-breakpoint
CREATE TABLE `profiles` (
	`id` text PRIMARY KEY NOT NULL,
	`data` text NOT NULL,
	`version` integer DEFAULT 0 NOT NULL
);
--> statement-breakpoint
CREATE TABLE `trades` (
	`id` text PRIMARY KEY NOT NULL,
	`maker` text NOT NULL,
	`offered` text NOT NULL,
	`wanted` text NOT NULL,
	`status` text DEFAULT 'open' NOT NULL
);
--> statement-breakpoint
CREATE UNIQUE INDEX `one_open_offer_per_card` ON `trades` (`offered`) WHERE "trades"."status" = 'open';