#ifndef HELPERS_H
#define HELPERS_H

#define FlagDef(ID) (1LL << ((ID)-1))
#define HasFlag(flags, flag) (((flags) & (flag)) != 0)
#define HasNoFlag(flags, flag) (((flags) & (flag)) == 0)
#define AddFlag(flags, flag) ((flags) |= (flag))
#define RemoveFlag(flags, flag) ((flags) &= ~(flag))
#define ToggleFlag(flags, flag) ((flags) ^= (flag))
#define SetFlag(flags, flag, condition) ((condition) ? AddFlag(flags, flag) : RemoveFlag(flags, flag))

#define true 1
#define false 0

typedef char bool;

#endif //HELPERS_H
