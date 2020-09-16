/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
    return ~(x & y) & ~(~x & ~y);
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
    return (0x1<<31);
    //return 0x80000000;数太大了
}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */

//最大值，一般符号位是0，其他是1，所以+1后，符号位是1，其他是0，这样相异或，就是全1,全1的反是0，0取非是1（判断是否是-1的方法）
//除了最大值，-1也有同样的情况，但是+1后，最大值是变成另一个数，而-1是0，根据这个排除掉-1
int isTmax(int x) {
    int max = !(~(x ^ (x + 1)));
    int minus = !!(x + 1);
    return max & minus;
    //判断 x+1 == 0x80000000
    //return !(x+0x80000001);Illegal constant (0x80000001) (only 0x0 - 0xff allowed)    2.（1<<31）,但是没有<<
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
//不能直接使用0xAAAAAAAA，需要构造，然后让x与0xAAAAAAAA相与，如果奇数位都是1，结果是0xAAAAAAAA，再异或就是0了，否则就非0

int allOddBits(int x) {
    int a = 0xaa << 8;       //0xaa00
    int b = (a | 0xaa);      //0xaaaa
    int c = b | (b << 16);
    return !((x & c) ^ c);
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
    return ~x+1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
//将x减去0x30和0x3a，用移位取其符号位。
//-x就是～x+1
//第一个是0,所以一次取非
//第二个是-1（全1）,先取非变0,再取非变1
int isAsciiDigit(int x) {
    return !((x + ~0x30 + 1) >> 31) & !!((x + ~0x3a + 1) >> 31);
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
//取两次非 => 0还是0,非1变成1
//如果是1,～x+1变成-1,全1，与y&还是y
//如果是0,只要一次非就变成1了
int conditional(int x, int y, int z) {
    return ((~(!!x) + 1) & y ) | ((~(!x) + 1) & z);
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
//相等用异或操作
//与0做比较，用移位判断符号位
//y-x => y+~x+1

int isLessOrEqual(int x, int y) {
    int x_sign = !(x >> 31); //正数为1
    int y_sign = !(y >> 31);
    //如果同符号,y-x>0
    int a = !(x_sign ^ y_sign) & !((y + (~x + 1)) >> 31);//要先判断是否符号相同
    //如果不同符号，当y为正数的时候返回
    int b = (~(x_sign ^ y_sign) + 1) & y_sign;//如果是异号，返回y_sign
    //如果相等，返回1
    int c = !(x ^ y);
    return a | b | c;
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
//判断0,如果正数和负数的符号位都是0
//～0 = -1 全1,& 0x1 返回1
//其他数，符号位1,都是1,取补是0，返回0
int logicalNeg(int x) {
    return (~(x>>31)&0x1)&(~((~x+1)>>31)&0x1);
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
    //两种特殊情况0、-1
    //0是全0，-1是全1，～-1 = 0

    int zero = (!x << 31) >> 31;//左移变成1000000...，然后再右移，变成111111...，直接右移，1就移出去了
    int minus = (!~x << 31 ) >> 31;
    //如果是负数，转成反码即可
    //与-1(全1)异或，可以变成反码,因为1遇1变0，0遇1变1；
    int number = x ^ (x >> 31);
    //因为是要判断共有多少位，采用二分算法
    //32分成16
    //如果有16位，两次取非结果为1，shift16 = 16；如果没有16位，两次取非结果为0，shift16=0.
    int shift16,shift8,shift4,shift2,shift1,sum;
    shift16 = ((!!(number >> 16)) << 4);
    //如果有16位，就将后面的截断（因为已经判断出来了）,否则的话，没有，就继续，没有动作。
    number = number >> shift16;
    shift8 = ((!!(number >> 8)) << 3);
    number = number >> shift8;
    shift4 = ((!!(number >> 4)) << 2);
    number = number >> shift4;

    shift2 = ((!!(number >> 2)) << 1);
    number = number >> shift2;

    shift1 = ((!!(number >> 1)));
    number = number >> shift1;

    //加2是因为，指针本身+（前面的符号位+1）
    sum = shift1 + shift2 + shift4 + shift8 + shift16 + 2;
    return (zero & 1) | (~zero & ((minus & 1 ) | (~minus & sum)));
}
//float
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */

//用单精度浮点来表示，但是传的是unsigned int，第一个符号位不需要管了
unsigned floatScale2(unsigned uf) {     //第一个 符号位 | 中间八个 指数部分 | 最后二十三个 尾数部分
    unsigned exp = (0x7F800000 & uf) >> 23;//取出指数部分
    unsigned sig = (0x7FFFFF & uf);//取出尾数部分

    //NaN是exp为255，但是sig不为0的
    //根据错误提示，当无穷大时，返回本身
    if(exp == 255){
        return uf;
    }else if(exp == 0){//非规格化
        return((uf & 0x80000000) + (sig << 1));
    }else{//非规格化
        return((uf & 0x80000000) + ((exp + 1) << 23) + sig);
    }
}
//规格化浮点数具有惟一的表示形式，而且在计算机中尾数能获得最大的有效数字，所以在一般的计算机中选用规格化浮点运算。

/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
////实参是unsigned的机器数，是一个浮点数，输出成对应转换的int型
int floatFloat2Int(unsigned uf) {
    int exp = (uf >> 23) & 0xff;
    int frac = (uf & 0x7FFFFF);//尾数
    int sig = !!(uf >> 31);//符号位

    int E = exp - 127;

    if(E < 0){                   //如果0.几，直接就返回0了
        return 0;
    }else if(E > 31){            //如果超过了int的范围
        return 0x80000000u;
    }else{
        frac = frac | (1 << 23);  //加上默认的1
        if(E < 23){
            frac = frac >> (23 - E);//因为后面尾数部分长度为23，能乘2^n，才能变成整数，不然就是小数部分，直接不要了就好
        }else{
            frac = frac << (E - 23);//因为后面尾数部分都乘出来了，所以，还有E，的话，就可以尾数*(E - 23)
        }
    }
    if(sig){
        return -frac;
    }else{
        return frac;
    }
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
//就是表示2^x,算出来的int值，然后用浮点数表示，输出该浮点数的机器数
unsigned floatPower2(int x) {
    unsigned INF = 0xff << 23; //阶码全1，尾数全0，符号位为0，是正无穷大
    int e = 127 + x;    // 得到阶码
    if (x < 0) // 阶数小于0直接返回0
        return 0;
    if (e >= 255) // 阶码>=255直接返回INF
        return INF;
//23是因为，阶码部分，就是这里
//0 | e | 全0

    return e << 23;//因为默认尾数是1.0，所以左移到23哪里，就是直接2^e

    // 直接将阶码左移23位，尾数全0，规格化时尾数隐藏有1个1作为底数
}
