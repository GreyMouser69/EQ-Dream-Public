local mq = require('mq')
local client = mq.TLO.EverQuest.Path() or ''
local root = mq.configDir .. '/tavern-live'
local pid = tonumber(mq.TLO.EverQuest.PID())
local path = root .. '/request-' .. tostring(pid) .. '.json'
local marker = root .. '/open-' .. tostring(pid)
local pending, requested, character = false, 0, ''
local function quote(s) return '"' .. s:gsub('\\','\\\\'):gsub('"','\\"') .. '"' end
local function write(action, token)
    local out=io.open(path..'.tmp','w')
    if not out then print('[Tavern Duels] Run the updated Solo Assist installer first.');return false end
    out:write('{"pid":'..pid..',"action":'..quote(action)..',"character":'..quote(character)..',"client":'..quote(client)..',"token":'..quote(token or '')..'}')
    out:close();os.remove(path);return os.rename(path..'.tmp',path)
end
mq.event('TavernLiveAccess','[Tavern Access] #1#',function(_,token)
    if not pending or mq.TLO.MacroQuest.GameState()~='INGAME' or mq.TLO.Me.CleanName()~=character then return end
    if #token~=64 or not token:match('^[a-f0-9]+$') then return end
    pending=false
    if write('open',token) then
        mq.cmdf('/exec "%s/../TavernDuels/TavernLive.exe" "%d" bg',mq.configDir,pid)
    end
end)
mq.bind('/tavern',function(command)
    local open=io.open(marker,'r');if open then open:close() end
    if command=='close' or open then pending=false;write('close');return end
    if mq.TLO.MacroQuest.GameState()~='INGAME' then print('[Tavern Duels] Log into EQ Dream first.');return end
    if pending then return end
    character=mq.TLO.Me.CleanName();pending=true;requested=os.time()
    mq.cmd('/say !tavern')
end)
print('[Tavern Duels] Ready. Type /tavern to play.')
while true do
    mq.doevents()
    if character~='' and (mq.TLO.MacroQuest.GameState()~='INGAME' or mq.TLO.Me.CleanName()~=character) then write('close');character='';pending=false end
    if pending and os.time()-requested>10 then pending=false;print('[Tavern Duels] No sign-in response. Try /tavern again, or tell a GM.') end
    mq.delay(250)
end
