import {authenticatedPlayer} from '@/lib/identity';
import {db} from '@/lib/db';
import {gameService} from '@/lib/game-service';
export async function POST(req:Request){
 try{
  const origin=req.headers.get('origin');
  if(origin&&origin!==new URL(req.url).origin)return Response.json({error:'Invalid origin'},{status:403});
  const user=await authenticatedPlayer(req.headers);
  if(!user)return Response.json({error:'Sign in to play.'},{status:401});
  const result=await gameService(db(),user,await req.json());
  return Response.json(result,{headers:{'Cache-Control':'no-store'}});
 }catch(e){return Response.json({error:e instanceof Error?e.message:'Unable to load game.'},{status:400,headers:{'Cache-Control':'no-store'}});}
}
