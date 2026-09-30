import type { Metadata } from "next";
import "./globals.css";

export const metadata: Metadata = {
  title: "EQ Dream: Tavern Duels",
  description: "An EverQuest-inspired card game. Build a deck and take a seat at the tavern.",
  other: {
    "codex-preview": "development",
  },
  icons: {
    icon: "/favicon.svg",
    shortcut: "/favicon.svg",
  },
};

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html lang="en">
      <body className="antialiased">{children}</body>
    </html>
  );
}
