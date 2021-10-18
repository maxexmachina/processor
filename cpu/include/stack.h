#ifndef STACK_H
#define STACK_H

#include <sys/types.h>
#include <limits.h>

#ifndef __FUNCTION_NAME__
    #ifdef WIN32   //WINDOWS
        #define __FUNCTION_NAME__   __FUNCTION__  
    #else          //*NIX
        #define __FUNCTION_NAME__   __func__ 
    #endif
#endif

#define DEBUG_MODE 3 

//------------------ User type -------------

//! String constant for name of the user type for debug output
static const char *typeName = "long long int";

//! Typedef for universal interface
typedef long long int elem_t;

//------------------------------------------------------------ 
//! User defined function that formats their data type to a string for debug output
//!
//! @param[in]  instance    Pointer to user type instance to format
//!
//! @return String version of the user type instance on success, nullptr on failure
//------------------------------------------------------------ 
char *formatInstance(const elem_t *instance);

//------------------------------------------------------------ 
//! User defined function that returns a poisoned instance of their type for debug
//!
//! @return A poisoned instance of the user type for debug
//------------------------------------------------------------ 
elem_t getPoisonedInstance();

//------------------------------------------------------------ 

//! Constant used by stack destructor to designate bytes it has already freed
static const int STK_DATA_POISON = 0x0F;
//! Constant for stack size used to prevent double destructor calls
static const size_t STK_SIZE_POISON = UINT_MAX; 

//! Encapsulates errors that can occur with the stack
enum StkError : int {
    STK_NO_ERR = 0, /**< No error */
    STK_NULL = 1, /**< Stack pointer is null */
    STK_INV_SIZE = 2, /**< Stack size has a value of STK_SIZE_POISON */ 
    STK_CAP_OVERFL = 3, /**< Stack size is more than stack capacity */
    STK_UNDERFL = 4, /**< Trying to pop from an empty stack */
    STK_NOMMRY = 5, /**< Memory allocation error */
#if DEBUG_MODE > 1
    STK_CAN_RGT = 6,
    STK_CAN_LFT = 7,
    STK_DATA_CAN_LFT = 8,
    STK_DATA_CAN_RGT = 9,
#endif
#if DEBUG_MODE > 2
    STK_HASH_FLR = 10,
#endif
};

//! Encapsulates info about function call for debug
struct callInfo {
    const char *funcName; /**< Name of the calling function */
    const char *file; /**< File where the function call happened */
    int line; /**< Line of the function call in the file */
};

#if DEBUG_MODE > 1
typedef unsigned long long canary_t;
const canary_t canaryVal = 3735927486; // 0xDEADBABE 
#endif

//! Declares a stack data structure as a struct
struct Stack {
#if DEBUG_MODE > 1
    canary_t canaryLeft;
#endif

    size_t size; /**< Current number of elements in the stack */
    size_t capacity; /**< Current full capacity of the array */ 
    size_t elemSize; /**< Size of the stored type in bytes */
    void *data; /**< Pointer to the stack data array */

#if DEBUG_MODE > 0
    const char *typeName; /**< String name of the data type stored in the stack for debug output */

    const char *ctorCallFuncName; /**< Function that called the stack constructor */
    const char *ctorCallFile; /**< File of the constructor call */
    int ctorCallLine; /**< Line of the constructor call */
#endif

#if DEBUG_MODE > 2
    size_t hash;
#endif

#if DEBUG_MODE > 1
    canary_t canaryRight;
#endif
};

#if DEBUG_MODE > 2
size_t hashStr(const char* s);

size_t hashData(void *data, size_t size);

size_t StackHash(Stack *stack);
#endif

//! Default stack capacity
const size_t DEFAULT_STACK_CAPACITY = 10;

#if DEBUG_MODE > 0
//------------------------------------------------------------ 
//! Checks if the given stack is valid
//!
//! @param[in]  stack   Pointer to the stack to check
//!
//! @return 0 if the stack is valid, one of the values of the StkError otherwise
//------------------------------------------------------------ 
int StackError(Stack *stack);

#if DEBUG_MODE > 1
canary_t *getDataCanaryLeft(Stack *stack);

canary_t *getDataCanaryRight(Stack *stack);
#endif

//------------------------------------------------------------ 
//! Writes given error code in a string form to the log file
//! 
//! @param[in]  err Error code that corresponds to one of the values in StkError enum
//!
//! @return 1 on success, 0 on failure
//------------------------------------------------------------ 
int writeErrCode(int err);

#endif

//------------------------------------------------------------ 
//! Performs a bytewise comparison of two instances of the same type
//!
//! @param[in]  elem1   Pointer to the first element
//! @param[in]  elem2   Pointer to the second element
//! @param[in]  size    Size of the type
//!
//! @return true if all bytes are equal, false otherwise
//------------------------------------------------------------ 
bool isEqualBytes(const void *elem1, const void *elem2, size_t size);

//------------------------------------------------------------ 
//! Returns the adress of the element at index in an arbitrary array
//!
//! @param[in]  start   Start of the array
//! @param[in]  index   Index of the element
//! @param[in]  size    Size of the type held by the array
//!
//! @return Address of the element at index in the array
//------------------------------------------------------------ 
void *getIndexAdress(void *start, size_t index, size_t size);

#if DEBUG_MODE > 0
//------------------------------------------------------------ 
//! Prints all information about the stack to the log file
//!
//! @param[in]  stack   Pointer to the stack
//! @param[in]  reason  String with reason for stack dump
//! @param[in]  info    Struct with information about function call location
//! @param[in]  stkName String name of the stack
//!
//! @return 1 on success, 0 on failure
//------------------------------------------------------------ 
int StackDump_(Stack *stack, const char *reason, callInfo info, const char *stkName);

#endif

//------------------------------------------------------------ 
//! Constructs the stack of given type with given capacity
//!
//! @param[out]  stack      Pointer to stack
//! @param[in]  elemSize    Size of the stored data type in bytes  
//! @param[in]  capacity    Desired capacity of the stack
//! @param[in]  info        Information about the constructor function call    
//!
//! @return 1 on success, 0 on failure
//------------------------------------------------------------ 
int StackCtor_(Stack *stack, size_t elemSize, size_t capacity, callInfo info);

//------------------------------------------------------------ 
//! Destroys the stack, freeing all allocated dynamic memory 
//! 
//! @param[in]  stack   Pointer to stack
//!
//! @note In debug mode sets all bytes of data array to STK_DATA_POISON, size to STK_SIZE_POISON, sets data pointer to 13
//------------------------------------------------------------ 
void StackDtor(Stack *stack);

//------------------------------------------------------------ 
//! Pops an element from the stack
//!
//! @param[in]  stack   Pointer to stack
//! @param[out] dest    Pointer to memory area where to copy the popped element
//! @param[in]  err     Pointer to variable that will store the error code
//------------------------------------------------------------ 
void StackPop(Stack *stack, void *dest, int *err = nullptr);

//------------------------------------------------------------ 
//! Pushes an element onto the stack
//!
//! @param[in]  stack   Pointer to stack
//! @param[in]  src     Pointer to memory area where the value to push is stored 
//! @param[in]  err     Pointer to variable that will store the error code
//------------------------------------------------------------ 
void StackPush(Stack *stack, void *src, int *err = nullptr);

//------------------------------------------------------------ 
//! Dynamically resizes the given stack
//!
//! @param[in]  stack   Pointer to stack
//! @param[in]  size    Desired new size
//!
//! @return 1 on success, 0 on failure
//------------------------------------------------------------ 
int StackResize(Stack *stack, size_t size);

void StackTop(Stack *stack, void *dest, int *err=nullptr);

#define ASSERT_OK(STACK)                            \
    do {                                            \
        int ret = StackError(STACK);                \
        if (ret != 0) {                             \
            StackDump(STACK, "ASSERT_OK failed");   \
            assert(!"Invariant failure");           \
        }                                           \
    } while (0)

#define StackCtor(stack, elemSize, capacity)    \
do {                                           \
    callInfo inf = {};                         \
    inf.funcName = __FUNCTION_NAME__;          \
    inf.file = __FILE__;                       \
    inf.line = __LINE__;                       \
    StackCtor_(stack, elemSize, capacity, inf); \
} while (0)

#define StackDump(stack, reason)      \
do {                                           \
    callInfo inf = {};                         \
    inf.funcName = __FUNCTION_NAME__;          \
    inf.file = __FILE__;                       \
    inf.line = __LINE__;                       \
    StackDump_(stack, reason, inf, #stack);   \
} while (0)

#endif
