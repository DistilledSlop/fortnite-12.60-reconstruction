"""Decision-only lift of the resolver's empty-normalized-input path.

Context methods must implement the original external calls, in call order:
  mode_object()                         0x140680300(0), null in this image
  mode_value(obj)                       obj virtual call at vtable +0x178
  branch_after_plus_to_slash()          0x142c8ff00 + 0x10, replace '+' with '/'
  tencent_flag()                        bool returned by 0x141ed9070
  find_branch(branch, needle)           0x142bace90(branch,needle,1,0,-1)
                                        return integer position or -1
The search helper's exact text semantics remain the context's responsibility.
This proposal emits log events rather than performing the original log calls;
it does not implement allocations, callback binding, or the following lookup.
"""


def select_default(ctx, once, log_verbosity):
    obj = ctx.mode_object()
    if obj is None:
        mode = 'Prod'
    elif ctx.mode_value(obj) == 1:
        mode = 'Stage'
    elif ctx.mode_value(obj) == 0:
        mode = 'GameDev'
    else:
        mode = 'Prod'

    branch = ctx.branch_after_plus_to_slash()
    is_ue4 = branch == 'UE4'  # exact UTF-16 equality in the resolver
    tencent = bool(ctx.tencent_flag())  # invoked even when mode is Prod
    events = []
    name = ''
    found = lambda needle: ctx.find_branch(branch, needle) != -1

    if mode == 'Prod':
        if not is_ue4 and not found('//Fortnite/Main') and not found('//Fortnite/Dev'):
            name = 'Fortnite'
        else:
            name = 'DevPlaytest'
    elif mode == 'Stage':
        name = 'CertStage'
    elif mode == 'GameDev':
        if found('//Fortnite/Release-3.5.CN'):
            name = 'FeatureALatest'
        elif not is_ue4 and not found('//Fortnite/Main') and not found('//Fortnite/Dev') and not found('//UE4/Fortnite-Staging'):
            if found('//Fortnite/Release'):
                name = 'ReleaseTesting'
            elif found('//UE4/Release'):
                name = 'UEReleaseTesting'
            elif found('//UE4/'):
                name = 'MainTesting'
            elif log_verbosity >= 2:
                events.append(('undetermined',))
        elif found('//Fortnite/Dev-FeatureA'):
            name = 'FeatureALatest'
        elif found('//Fortnite/Dev-FeatureB') or tencent:
            name = 'FeatureBLatest'
        elif found('//Fortnite/Dev-Athena'):
            name = 'DevAthenaLatest'
        else:
            name = 'DevLatest'

    if is_ue4 and not once.get('ue4_warning_issued', False):
        if log_verbosity >= 3:
            events.append(('guessing', name, branch))
        once['ue4_warning_issued'] = True
    if log_verbosity >= 7:
        events.append(('using_default', name))
    return name, events
