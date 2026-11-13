# Legacy GPU contract input

These three files retain their original bytes and pinned inventory hashes.
They are compatibility inputs for historical checkpoint/FP32 verification tools,
not the active completion contract. Current acceptance is
[Native GPU v4](../native-gpu-acceptance-v4/acceptance-contract.json).
The historical predicate text inside this frozen contract names its old path;
use an explicit current contract path when invoking a tool.
