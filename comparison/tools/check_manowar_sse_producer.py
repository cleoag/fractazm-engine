"""Reuse the strict caller checker for the sole uncounted SSE producer class."""
import check_manowar_hybrids as checker

base = checker.consumed_parameter_differences
def consumed(name, pascal, cpp):
    if name != '_IntPowRotateC':
        return base(name, pascal, cpp)
    if len(pascal) != 512 or len(cpp) != 1024:
        raise ValueError('invalid parameter buffer length')
    # Immutable CODE reads all negative options and constants [PVar,PVar+24).
    different = [i-256 for i,(a,b) in enumerate(zip(pascal,cpp[256:768])) if a != b]
    return different, [i for i in different if i < 24]

if __name__ == '__main__':
    checker.consumed_parameter_differences = consumed
    raise SystemExit(checker.main())
