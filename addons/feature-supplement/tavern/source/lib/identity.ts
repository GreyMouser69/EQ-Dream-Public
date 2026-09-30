// These headers are supplied by the Sites authentication dispatcher.
// Never accept identity from JSON, query parameters, or browser storage.
export async function authenticatedPlayer(headers:Headers):Promise<string|null>{
 const id=headers.get('oai-authenticated-user-id')?.trim();
 if(id)return id;
 const email=headers.get('oai-authenticated-user-email')?.trim().toLowerCase();
 if(!email||!email.includes('@'))return null;
 const digest=await crypto.subtle.digest('SHA-256',new TextEncoder().encode(email));
 return 'sites-email:'+Array.from(new Uint8Array(digest),b=>b.toString(16).padStart(2,'0')).join('');
}
