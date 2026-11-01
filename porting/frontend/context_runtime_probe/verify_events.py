"""Strict checks of actual observer trace; raises ValueError for any missing evidence."""
import re
from collections import Counter

def require(ok, why):
    if not ok: raise ValueError(why)

def parse(log):
    result=[]
    for line in log.splitlines():
        match=re.search(r'CONTEXT_(OBS|CASE|LIFE) ([\w]+)(.*)$',line)
        if not match: continue
        kind,event,tail=match.groups(); pairs=tail.strip().split(); fields={}
        for pair in pairs:
            require('=' in pair,'malformed field')
            key,val=pair.split('=',1);require(key and val and key not in fields,'duplicate or empty field');fields[key]=val
        result.append((kind,event,fields))
    return result

def segment(e,kind,start,end):
    starts=[i for i,x in enumerate(e) if x[0:2]==(kind,start)]; ends=[i for i,x in enumerate(e) if x[0:2]==(kind,end)]
    require(len(starts)==len(ends)==1 and starts[0]<ends[0],kind+' boundary '+start)
    return e[starts[0]+1:ends[0]]

def registry(log):
    e=parse(log); owners=[x[2] for x in e if x[:2]==('OBS','owner')]
    require(len(owners)==1 and owners[0]['manager']!='0' and owners[0]['gi']!='0','unique first owner')
    identity=[x[2] for x in e if x[:2]==('CASE','identity')]
    require(len(identity)==1 and identity[0]['gi']==owners[0]['gi'],'first owner matches diagnostic GI')
    state={}; player=None
    for phase in (1,2):
        seq=[x for x in segment(e,'CASE',f'create{phase}_begin',f'create{phase}_end') if x[0]=='OBS']
        index=0; made=[]; posts=[]
        while index<len(seq):
            _,event,f=seq[index]
            if event=='post': posts.append(f['object']);index+=1;continue
            require(event=='insert' and not posts,'insertion must precede all post init')
            cls=f['class']; p=f['player'];player=player or p
            require(p==player and cls!='0' and f['old']==state.get(cls,'0'),'prior slot mismatch')
            prior=state.copy();state[cls]='0';index+=1
            require(index<len(seq) and seq[index][1]=='init_begin','missing early initialize')
            b=seq[index][2];require(b['player']==p and b['class']==cls and b['self']=='0' and b['slot']=='1' and b['object']!='0','early self')
            obj=b['object'];index+=1;peers={}
            while index<len(seq) and seq[index][1]=='peer':
                q=seq[index][2];require(q['player']==p and q['creating']==cls and q['class'] not in peers,'peer identity');peers[q['class']]=q['value'];index+=1
            require(peers==state,'early peer snapshot differs from current slots')
            require(index<len(seq) and seq[index][1]=='init_end' and seq[index][2]['object']==obj,'missing matching initialize end');index+=1
            require(index<len(seq) and seq[index][1]=='stored' and seq[index][2]=={'player':p,'class':cls,'value':obj},'store mismatch');index+=1
            if phase==2: require(cls in prior and prior[cls]!='0' and prior[cls]!=obj,'replacement')
            state[cls]=obj;made.append(obj)
        require(len(made)>=2 and Counter(posts)==Counter(made),'post init after all creations')
    seq=[x for x in segment(e,'CASE','remove_begin','remove_end') if x[0]=='OBS'];removed=[];i=0
    while i<len(seq) and seq[i][1]=='removing':
        o=seq[i][2]['object'];i+=1
        require(i<len(seq) and seq[i][1]=='kill' and seq[i][2]['object']==o and seq[i][2].get('pending')=='1','remove/kill sequence');removed.append(o);i+=1
    require(Counter(removed)==Counter(state.values()) and i==len(seq)-1 and seq[i][1]=='remove_map' and seq[i][2]['player']==player and seq[i][2].get('remains')=='0','remove map ordering')
    gc=segment(e,'CASE','gc_begin','gc_end');begins=[x for x in e if x[:2]==('CASE','gc_begin')];obj=begins[0][2]['object'];deinit=[x for x in gc if x[0]=='OBS' and x[1] in ('deinit_begin','deinit_end') and x[2].get('object')==obj];require(obj in state.values() and [x[1] for x in deinit]==['deinit_begin','deinit_end'],'actual destruction and deinitialize of removed object')
    require(identity[0]['player']==player,'diagnostic player identity')
    return {'classes':len(state),'owner':owners[0],'player':player}

def copied_dispatch(events, callback, world, player, live):
    require(events and events[0][1]=='copy_begin' and events[-1][1]=='copy_end','copy boundaries')
    i=1;seen=set();selected=None
    while i<len(events)-1:
        require(events[i][1]=='copy_player','copied player begin');p=events[i][2]['player'];require(p not in seen,'duplicate copied player');seen.add(p);i+=1
        expected=[]
        while i<len(events) and events[i][1]=='copy_expected':
            f=events[i][2];require(f['player']==p,'copied player identity');expected.append(f['object']);i+=1
        actual=[]
        while i<len(events) and events[i][1]==callback:
            f=events[i][2];require(f['world']==world,'dispatch world');actual.append(f['object']);i+=1
        require(actual==expected,'each copied context dispatched exactly once in order')
        require(i<len(events) and events[i][1]=='copy_player_end' and events[i][2]['player']==p,'copied player end');i+=1
        if p==player:selected=expected
    require(i==len(events)-1 and selected is not None and Counter(selected)==Counter(live),'selected player copied live context multiset')

def lifecycle(log):
    e=parse(log);owner=[x[2] for x in e if x[:2]==('OBS','owner')]
    require(len(owner)==1 and owner[0]['gi']!='0','unique first GI')
    begin=[x[2] for x in e if x[:2]==('LIFE','begin')]
    require(len(begin)==1 and begin[0]['gi']==owner[0]['gi'],'lifecycle first owner GI')
    created=segment(e,'LIFE','begin','created')
    live=[x[2]['value'] for x in created if x[:2]==('OBS','stored') and x[2]['player']==begin[0]['player']]
    require(len(live)>=2 and len(set(live))==len(live) and all(x!='0' for x in live),'lifecycle created instances')
    body=segment(e,'LIFE','created','end');require(body,'empty lifecycle')
    obs=[x for x in body if x[0]=='OBS'];good_ready=False;good_cleanup=False;pending={};i=0
    while i<len(obs):
        _,event,f=obs[i]
        if event=='world_init':
            require(i+1<len(obs) and obs[i+1][1]=='pending' and obs[i+1][2]['world']==f['world'],'world init then pending')
            item=obs[i+1][2];pending[item['name']]=item['world'];i+=2;continue
        if event=='deferred':
            j=i+1
            while j<len(obs) and obs[j][1]!='deferred_end':j+=1
            require(j<len(obs) and obs[j][2]['name']==f['name'],'deferred end')
            block=obs[i+1:j]
            if block:
                require(len(block)>=2 and block[0][1]=='consumed' and block[1][1]=='owner_match','deferred consume and owner order')
                c=block[0][2];match=block[1][2]
                require(c['name']==f['name'] and c['remains']=='0' and pending.get(c['name'])==c['world'],'consumed pending world')
                pending.pop(c['name']);require(match['world']==c['world'] and match['first']==owner[0]['gi'],'deferred owner identity')
                calls=block[2:]
                if match['match']=='1':
                    require(match['actual']==owner[0]['gi'],'matched deferred owner');copied_dispatch(calls,'ready',c['world'],begin[0]['player'],live);good_ready=True
                else:require(not calls,'unmatched deferred dispatched')
            i=j+1;continue
        if event=='world_cleanup':
            j=i+1
            while j<len(obs) and obs[j][1]!='world_cleanup_end':j+=1
            require(j<len(obs) and obs[j][2]['world']==f['world'],'cleanup end')
            block=obs[i+1:j];require(block and block[0][1]=='owner_match','cleanup owner order');match=block[0][2]
            require(match['world']==f['world'] and match['first']==owner[0]['gi'],'cleanup owner identity')
            calls=block[1:]
            if match['match']=='1':
                require(match['actual']==owner[0]['gi'],'matched cleanup owner');copied_dispatch(calls,'cleanup_dispatch',f['world'],begin[0]['player'],live);good_cleanup=True
            else:require(not calls,'unmatched cleanup dispatched')
            i=j+1;continue
        require(event not in ('pending','consumed','ready','cleanup_dispatch','owner_match','deferred_end','world_cleanup_end'),'orphan world event')
        i+=1
    require(good_ready and good_cleanup,'matched real deferred and cleanup dispatch required')
    tail=segment(e,'LIFE','end','removed');obs=[x for x in tail if x[0]=='OBS']; require(obs and obs[-1][1]=='remove_map' and obs[-1][2]['player']==begin[0]['player'] and obs[-1][2].get('remains')=='0','lifecycle map removal');i=0;removed=[]
    while i<len(obs)-1:
        require(i+1<len(obs) and obs[i][1]=='removing' and obs[i+1][1]=='kill' and obs[i][2]['object']==obs[i+1][2]['object'] and obs[i+1][2].get('pending')=='1','lifecycle remove/kill');removed.append(obs[i][2]['object']);i+=2
    require(i>0 and i==len(obs)-1 and Counter(removed)==Counter(live),'lifecycle removal count')
    return {'owner':owner[0]}
