#!/usr/bin/env python3
#
# Copyright 2023 The Dawn Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Script for updating the active milestones for the Dawn project.

This is largely based on the Chromium equivalent in
chromium/src/infra/config/scripts/milestones.py.

To activate a new branch, run the following from the root of the repo (where MMM
is the milestone number and BBBB is the branch number):
```
infra/config/scripts/milestones.py activate --milestone MMM --branch BBBB
infra/config/global/main.star
```

To deactivate a branch, run the following from the root of the repo (where MMM
is the milestone number):
```
infra/config/scripts/milestones.py deactivate --milestone MMM
infra/config/global/main.star
```
"""

import argparse
import json
import os

INFRA_CONFIG_GLOBAL_DIR = os.path.realpath(
    os.path.join(os.path.dirname(__file__), '..', 'global'))


def parse_args():
    parser = argparse.ArgumentParser(
        description='Update the active milestones for the Dawn project')
    parser.set_defaults(func=None)
    parser.add_argument('--milestones-json',
                        default=os.path.join(INFRA_CONFIG_GLOBAL_DIR,
                                             'milestones.json'),
                        help='Path to the milestones.json file to modify')

    subparsers = parser.add_subparsers()

    activate_parser = subparsers.add_parser(
        'activate', help='Add an additional active milestone.')
    activate_parser.set_defaults(func=cmd_activate)
    activate_parser.add_argument(
        '--milestone',
        required=True,
        type=int,
        help='The milestone identifier/release channel number')
    activate_parser.add_argument(
        '--branch',
        required=True,
        help='The branch name, which should correspond to a ref in refs/heads')

    deactivate_parser = subparsers.add_parser(
        'deactivate', help='Remove an active milestone')
    deactivate_parser.set_defaults(func=cmd_deactivate)
    deactivate_parser.add_argument(
        '--milestone',
        required=True,
        type=int,
        help='The milestone identifier/release channel number')

    args = parser.parse_args()
    if not args.func:
        parser.error('No sub-command specified')
    return args


def _sort_milestones(milestones):
    # We have to manually sort here instead of relying on
    # json.dump(..., sort_keys=True) later since we only want to sort the top
    # level keys, not all keys.
    milestones = {
        str(k): milestones[str(k)]
        for k in sorted([int(s) for s in milestones])
    }
    return milestones


def add_milestone(milestones, milestone_num, branch):
    if str(milestone_num) in milestones:
        raise RuntimeError('Milestone %d already exists' % milestone_num)

    milestones[str(milestone_num)] = {
        'name': f'm{milestone_num}',
        'chromium_project': f'chromium-m{milestone_num}',
        'ref': f'refs/heads/{branch}',
        'platforms': [
            'linux',
            'mac',
            'win',
        ],
    }

    return _sort_milestones(milestones)


def remove_milestone(milestones, milestone_num):
    if str(milestone_num) not in milestones:
        raise RuntimeError('Milestone %d does not exist' % milestone_num)
    del milestones[str(milestone_num)]
    # Not strictly necessary, but returning a value keeps this consistent with
    # add_milestone.
    return milestones


def cmd_activate(args):
    with open(args.milestones_json) as infile:
        milestones = json.load(infile)
    milestones = add_milestone(milestones, args.milestone, args.branch)
    with open(args.milestones_json, 'w') as outfile:
        json.dump(milestones, outfile, indent=4)


def cmd_deactivate(args):
    with open(args.milestones_json) as infile:
        milestones = json.load(infile)
    milestones = remove_milestone(milestones, args.milestone)
    with open(args.milestones_json, 'w') as outfile:
        json.dump(milestones, outfile, indent=4)


def main():
    args = parse_args()
    args.func(args)


if __name__ == '__main__':
    main()
