#!/bin/bash
# usage: work/claude/finish_chunk.sh NN   (run from the package dir): record proposals, mirror outputs into the repo, commit.
set -e
N=$1; R=/opt/fortnite/FortniteClient-12.60-CL-13477524; D=$R/porting/server_scope/local_model
python3 -I work/claude/record_chunk.py $N | sort | uniq -c
rsync -a --exclude='*-req*.json' work/claude/out/ $D/claude/
cp work/claude/{record_chunk.py,finish_chunk.sh,WORKER.md} $D/claude/
cd $R && git add porting/server_scope/local_model && git -c user.name="Claude Sonnet 5.5" -c user.email=noreply@anthropic.com commit -q --author="Claude Sonnet 5.5 <noreply@anthropic.com>" -m "Record chunk $N native proposals

Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>" && git log -1 --format='%h %s'
