# Local functions <br> && <br> Function literals

### N3678 and N3679


## Motivation

```c
 void async(void* data, void (*callback)(int result, void* data));

    struct start_capture {
        int value;
    };

    static void start_callback(int result, void* data) {
        struct start_capture* capture = data;
        free(capture);
    }

    void start() {
       struct start_capture* capture = calloc(1, sizeof *capture);
       async(capture, start_callback);
    }
```

## do this, then that...

```csmall
void part1_async(void* data, void (*callback)(int result, void* data));
    void part2_async(void* data, void (*callback)(int result, void* data));

    struct part1_capture {
        int value;
    };

    struct part2_capture {
        char ch;
    };

    static void part2_complete(int result, void* data);

    static void part1_complete(int result, void* data) {
       struct part1_capture* capture1 = data;

        struct part2_capture* capture2 = calloc(1, sizeof *capture2);
        part2_async(capture2, part2_complete);

        free(capture2);
    }

    static void part2_complete(int result, void* data) {
       struct part2_capture* capture2 = data;
       free(capture2);
    }

    void start() {
       struct part1_capture* capture1 = calloc(1, sizeof *capture1);
       part1_async(capture1, part1_complete);
    }
```

## Local functions
```c
void async(void* data, void (*callback)(int result, void* data));

    void start() {
       
       struct capture {
            int value;
        };

       static void callback(int result, void* data) {
            struct capture* capture = data;
            free(capture);
       }
       
       struct capture* capture = calloc(1, sizeof * capture);
       async(capture, callback);
    }
```

## Literal functions
```c
    void async(void* data, void (*callback)(int result, void* data));

    void start() {
       
       struct capture {
            int value;
       } capture = calloc(1, sizeof * capture);

       async(capture, (void (int result, void* data)) 
       {
          struct capture* capture = data;
          free(capture);
       });
    }
```

## Reusing captures

```c
int main()
{
    struct capture { int id; }* capture = calloc(1, sizeof * capture);
    login_async(capture,  (void (int id, void * p))
    {
        printf("login completed. id=%d\n", id);
        struct capture * cap1 = p;
        cap1->id = id;
        get_data_async(cap1 /*moved*/, (void (const char* email, void * data))
        {
            struct capture * cap2 = data;
            printf("your data='%s'  from id=%d\n", email, cap2->id);
            free(cap2);
        });
    });
}
```

## Local function syntax
```
block-item:
           ...
           <span style="color:blue">function-definition</span>
       
       function-definition:
           attribute-specifier-sequence opt declaration-specifiers declarator function-body
```

## Forward declarations

```c
#include <stdio.h>

int main() {
   void f();
   f();
   void f() { printf("local"); } 
}

void f() { printf("extern"); }
```

## GCC Nested function solution
```c
#include <stdio.h>

int main() {
   auto void f();
   f();
   /*auto*/ void f() { printf("local"); } 
}

void f() { printf("extern"); }
```
> See also: N3579 auto as a placeholder type specifier

## GCC Nested function solution
```c
#include <stdio.h>

int main() {
   void f();
   f();
   /*error: static declaration of 'f' follows non-static declaration*/
   void f() { printf("local"); } 
}

void f() { printf("extern"); }
```

## Alternative I (static)
```c
void f() { /*extern*/ }

int main() {
   
   /*local functions*/
   static void f(); /*local function declaration*/
   static void f() { }   

   /*GCC nested function*/
   auto int f2();
   int i = 1;              
   int f2() { return i; }
}
```

## Alternative II (hybrid)

```c
void f() { /*extern*/ }

int main() {
   static void f();
   void f() { }
      
   /*static*/ int f1() { return 0; }   

   int i = 1;              
   int f1() { return i; /*GCC extension*/ }   
}
```

## Alternative III (same as gcc)

```c
void f() { /*extern*/ }

int main() {
   auto void f();
   /*auto*/ void f() { }

   auto int f2();
   int i = 1;              
   int f2() { return i; /*GCC extension*/ }   
}
```

## Multiple forward declarations/definitions

```c
#include <stdio.h>

int main()
{
    int n = 1;
    auto typeof(int [n])* f();
    printf("%zu\n", _Countof(*f())); //1
    
    n = 2;
    auto typeof(int [n])* f();
    printf("%zu\n", _Countof(*f())); //2
    
    n = 3;
    auto typeof(int [n])* f(){
    }
    printf("%zu\n", _Countof(*f())); //3
}
```

##  Minimizing problems

- We cannot have a "forward declaration" after the definition.
- We should have only one "forward declaration" or none.
- A local function must have only one definition per scope. 
 
## Function Literal syntax
 
```
   postfix-expression: 
            ...      
            function-literal-definition
  
      function-literal-definition:
         ( attribute-specifier-sequence opt declaration-specifiers abstract-declarator ) 
              function-body 
       

       function-definition:
          attribute-specifier-sequence opt declaration-specifiers declarator function-body
```

> The abstract-declarator portion of a function literal definition must have a function type.

> Extra attribute-specifier-sequence may be necessary for [[unsequenced]] and similar


## Semantics
- The function literal is a function designator. (Behaves like a function, not a function pointer) 

```c
void main()
{
    (void (*pf1)(void)) = (void (void)){}; /* ok */
    (void (*pf2)(void)) = &(void (void)){}; /* ok */
    &(void (void)){} = 0;                  /* error: lvalue required */
}
```
## File scope function literals

```
auto f = (int (int a)){ return a * 2; }; /* ok */
```

> I don't have a use case for that at the moment.

## Labels

- Labels are not shared
- Statements are not shared (break; continue)

```c
int main() {
    L1:;
    (void (void)) {    
        goto L1; /* error: label 'L1' used but not defined */
    }();

   void local() {     
       goto L1; /* error: label 'L1' used but not defined */
   };
}
```

## Returning VM types

```
#include <stdio.h>
int main(){
    int n = 1;
    auto typeof(int [n])* local(void);

    n  = 2;
    typeof(int [n])* local(void) {
       return 0;
    }    
    
    n = 3;
    auto r = local();

    n = 4;
    printf("%zu", _Countof(*r)); //returns 2
}
```
> Following GCC implementation of nested functions

## Argument evaluation

```
#include <stdio.h>

int main() {
    int n = 1;

    void local(typeof(int[n])* p) 
    {
         printf("%zu", _Countof(*p)); //prints 3
    }

    n = 2;
    int a[n];
    
    n = 3;
    local(&a);
}
```

> Following GCC implementation of nested functions

## __func__

- The value of the string returned by __func__ is implementation-defined.

> GCC returns the function name for nested functions

> C++ returns "operator ()" in lambdas

## Scope

- Function literals and local functions have access to the enclosing scope at the point of its definition.

```
int main() {
    
    struct X {int i; };
    enum E {A};

    (void (void)) {    
        struct X x = {}; /* ok */
        x.i = A;         /* ok */
    }();

   void local() {     
     struct X x = {}; /* ok */
     x.i = A;         /* ok */
   };
}
```

## Automatic variables

Identifiers referring to automatic variables of an enclosing function 
cannot have their address resolved inside the body of a function literal or 
local function. If they have VM types, this restriction also apply to resolving their type. 

```

int main() {    
    int i = 2;
    void local() {    
        int j = sizeof(i); /* ok */
        int  k = i;        /* constraint violation */
        int *p = &i;   /* constraint violation */
    };   
}

void start(int n) {
    int a[n];
    void local() {    
        typeof(a) k;       /* constrain violation */
        int m = sizeof(a); /* constrain violation */
    };   
}
```

## Constants

The same restrictions that apply to automatic variables also apply here, but we may 
assume they can be read without having access to their address. 


```
int main() {
   constexpr int a = 1;
   const int b = 2;     /*N3693 Implicitly constexpr*/

   void local() {     
     int x = a;   /* ok */             
     int *p = &a; /*constrain violation*/
     
     n = b;       /* ok */             
     p = &b;      /*constrain violation*/
   };
}
```

> Originally left as a possible option, it can be included


## Non-automatic variables

```
static int g = 1;

int main() {
   static int i = 1;
   
   void local() {     
     int j = sizeof(i); /* ok */
     int k = i;         /* ok */
     int m = g;         /* ok */     
   };
}
```
> For synchronous code, static variables can be an alternative to captures

## Function literal emulation in GCC

```
int main() {
    ({int _(int a) { return a * 2; } _;})(2);
}
```

## Generic functions

```
#define SWAP(a, b)\
    (void (typeof(a)* arg1, typeof(b)* arg2)) { \
    typeof(a) temp = *arg1; *arg1 = *arg2; *arg2 = temp; \
    }(&(a), &(b))

int main() {
    int a = 1;
    int b = 2;

    SWAP(a, b);
    
    (void (typeof(a)* arg1, typeof(b)* arg2)) { 
        typeof(a) temp = *arg1;
        *arg1 = *arg2; 
        *arg2 = temp; 
    }(&(a), &(b));


    double da = 1.0;
    double db = 2.0;
    SWAP(da, db);
}
```
## Function Literal address
- Distinct function literals are not required to have unique addresses.
- local functions?


```
int main(){
    auto pf1 = (void ()) { return 1 + 1; };
    auto pf2 = (void ()) { return 2; };
    auto pf3 = (void ()) { return 2; };
    /* pf1 and pf2 and pf3 can have the same address */
}
```


##  Static variables inside function literals
- static variables inside function literals will generate distinct functions

```
int main() {
    auto pf1 = (void ()) { static int i = 0; };
    auto pf2 = (void ()) { static int i = 0; };
    assert(pf1 != pf2);
}
```

## Why not C++ lambda syntax?

- Keeps the grammar for functions and function literals in sync.
- Keeps the existing scope rules for return types and parameters.

## Key points

- Almost zero learning curve
- Existing practice
- Does not require trampolines or other hidden features.
- No forced capture strategy (by reference, by copy, stack, heap, etc.).
- Works with existing APIs that use void* callbacks
- Lifetime problems may exist, but they are not new
- Implementation is not complex


## Next steps

-  Improving the proposal, add wording
-  Deciding on forward-declaration syntax
-  Experimental implementation (Cake — missing VM type constraints)


# Thank You
Press ← → or Space to navigate.


## References

-    N3724: Discarded, https://www.open-std.org/JTC1/SC22/WG14/www/docs/n3724.pdf
-    N3622 Allow calling static inline within extern inline
-    N3579: auto as a placeholder type specifier, v2, https://www.open-std.org/JTC1/SC22/WG14/www/docs/n3579.htm
-    N2956: Unsequenced functions https://www.open-std.org/JTC1/SC22/WG14/www/docs/n2956.htm
-    N3693: Integer Constant Expression-Initialized const Integer-Typed Declarations are Implicitly constexpr https://www.open-std.org/jtc1/sc22/wg14/www/docs/n3693.htm
-    N3694: Functions with Data https://www.open-std.org/jtc1/sc22/wg14/www/docs/n3694.htm
-    N3654: Accessing the Context of Nested Functions https://www.open-std.org/jtc1/sc22/wg14/www/docs/n3654.pdf
-    Reddit: https://www.reddit.com/r/C_Programming/comments/1omrrra/closures_in_c_yes/
    http://cakecc.org/


