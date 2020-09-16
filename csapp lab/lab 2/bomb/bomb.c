/***************************************************************************
“二进制炸弹”是以目标代码文件的形式提供给学生的程序。运行时，它会提示用户键入6个不同的字符串。如果其中任何一个错误，炸弹“爆炸”，打印错误消息并将事件记录在分级服务器上。学生们必须“拆除”他们自己独特的炸弹分解和逆向工程程序，以确定6个字符串应该是什么。该实验室教学生们理解汇编语言，并迫使他们学习如何使用调试器。这也很有趣。CMU本科生中的传奇实验室。
这里有一个Linux/x86-64二进制炸弹，你可以自己试试。通知分级服务器的功能已被禁用，因此请随意引爆此炸弹而不受惩罚。如果你是一个拥有CS:APP帐户的讲师，那么你可以下载解决方案。
 * Dr. Evil's Insidious Bomb, Version 1.1
 * Copyright 2011, Dr. Evil Incorporated. All rights reserved.
 *
 * LICENSE:
 *
 * Dr. Evil Incorporated (the PERPETRATOR) hereby grants you (the
 * VICTIM) explicit permission to use this bomb (the BOMB).  This is a
 * time limited license, which expires on the death of the VICTIM.
 * The PERPETRATOR takes no responsibility for damage, frustration,
 * insanity, bug-eyes, carpal-tunnel syndrome, loss of sleep, or other
 * harm to the VICTIM.  Unless the PERPETRATOR wants to take credit,
 * that is.  The VICTIM may not distribute this bomb source code to
 * any enemies of the PERPETRATOR.  No VICTIM may debug,
 * reverse-engineer, run "strings" on, decompile, decrypt, or use any
 * other technique to gain knowledge of and defuse the BOMB.  BOMB
 * proof clothing may not be worn when handling this program.  The
 * PERPETRATOR will not apologize for the PERPETRATOR's poor sense of
 * humor.  This license is null and void where the BOMB is prohibited
 * by law.
 *
* Dr. Evil's Insidious Bomb, Version 1.1
* Copyright 2011, Dr. Evil Incorporated. All rights reserved.
*
* LICENSE:
*
* Dr. Evil Incorporated (the PERPETRATOR) hereby grants you (the
* VICTIM) explicit permission to use this bomb (the BOMB). This is a
* time limited license, which expires on the death of the VICTIM.
* The PERPETRATOR takes no responsibility for damage, frustration,
* insanity, bug-eyes, carpal-tunnel syndrome, loss of sleep, or other
* harm to the VICTIM. Unless the PERPETRATOR wants to take credit,
* that is. The VICTIM may not distribute this bomb source code to
* any enemies of the PERPETRATOR. No VICTIM may debug,
* reverse-engineer, run "strings" on, decompile, decrypt, or use any
* other technique to gain knowledge of and defuse the BOMB. BOMB
* proof clothing may not be worn when handling this program. The
* PERPETRATOR will not apologize for the PERPETRATOR's poor sense of
* humor. This license is null and void where the BOMB is prohibited
* by law.
*邪恶博士的阴险炸弹，1.1版
*版权所有2011，Dr.Viil Incorporated。版权所有。
*许可证：
*邪恶博士公司（犯罪人）特此授予你
*受害者）明确允许使用这个炸弹（炸弹）。这是一个
*有时间限制的许可证，在受害者死亡时到期。
*肇事者对损害、挫折不负任何责任，
*精神错乱，眼睛有毛病，腕管综合症，失眠或其他
*伤害受害者。除非犯罪者想获得荣誉，
*就是这样。受害者不得将此炸弹源代码分发给
*犯罪者的任何敌人。受害者不能调试，
*反向工程，运行“字符串”，反编译，解密，或使用任何
*其他技术，以获得知识和拆除炸弹。炸弹
*处理此程序时，不得穿校服。这个
*肇事者不会为肇事者的不良意识道歉
*幽默。本许可证在禁止使用炸弹的地方无效
*根据法律。
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "support.h"
#include "phases.h"

/* 
 * Note to self: Remember to erase this file so my victims will have no
 * idea what is going on, and so they will all blow up in a
 * spectaculary fiendish explosion. -- Dr. Evil
 *
* Note to self: Remember to erase this file so my victims will have no
* idea what is going on, and so they will all blow up in a
* spectaculary fiendish explosion. -- Dr. Evil
*自我提醒：记住要删除这个文件，这样我的受害者就没有了
*知道发生了什么，所以他们都会在
*壮观的恶魔般的爆炸。--邪恶博士
 */

FILE *infile;

int main(int argc, char *argv[])
{
    char *input;

    /* Note to self: remember to port this bomb to Windows and put a 
     * fantastic GUI on it.
     *
     * 赛尔夫注意：记住把炸弹放在窗户上
     * 很棒的图形用户界面。
     */

    /* When run with no arguments, the bomb reads its input lines
     * from standard input.
    当不带参数运行时，炸弹读取其输入行从标准输入。  */
    if (argc == 1) {  
	infile = stdin;
    } 

    /* When run with one argument <file>, the bomb reads from <file> 
     * until EOF, and then switches to standard input. Thus, as you 
     * defuse each phase, you can add its defusing string to <file> and
     * avoid having to retype it.
     *当用一个参数<file>运行时，炸弹从<file>读取
    直到EOF，然后切换到标准输入。因此，作为你
    分解每个阶段，可以将其分解字符串添加到<file>和
    避免重打。
     * */
    else if (argc == 2) {
	if (!(infile = fopen(argv[1], "r"))) {
	    printf("%s: Error: Couldn't open %s\n", argv[0], argv[1]);
	    exit(8);
	}
    }

    /* You can't call the bomb with more than 1 command line argument.
     *不能用一个以上的命令行参数调用炸弹。 */
    else {
	printf("Usage: %s [<input_file>]\n", argv[0]);
	exit(8);
    }

    /* Do all sorts of secret stuff that makes the bomb harder to defuse.
     *做各种让炸弹更难拆除的秘密活动。
     * */
    initialize_bomb();

    printf("Welcome to my fiendish little bomb. You have 6 phases with\n");
    printf("which to blow yourself up. Have a nice day!\n");

    /* Hmm...  Six phases must be more secure than one phase!六个阶段必须比一个阶段更安全！ */
    input = read_line();             /* Get input 获取输入    */
    phase_1(input);                  /* Run the phase 运行阶段   */
    phase_defused();                 /* Drat!  They figured it out! 该死！他们发现了！
				      * Let me know how they did it.告诉我他们是怎么做到的。 */
    printf("Phase 1 defused. How about the next one?\n");

    /* The second phase is harder.  No one will ever figure out
     * how to defuse this...
     *第二阶段比较困难。没人会知道的如何化解。。。
     * */
    input = read_line();
    phase_2(input);
    phase_defused();
    printf("That's number 2.  Keep going!\n");

    /* I guess this is too easy so far.  Some more complex code will
     * confuse people.
     * 我想到目前为止这太容易了。一些更复杂的代码迷惑人。
     * */
    input = read_line();
    phase_3(input);
    phase_defused();
    printf("Halfway there!\n");

    /* Oh yeah?  Well, how good is your math?  Try on this saucy problem!
     * 哦是吗？你的数学有多好？试试这个俏皮的问题！
     * */
    input = read_line();
    phase_4(input);
    phase_defused();
    printf("So you got that one.  Try this one.\n");
    
    /* Round and 'round in memory we go, where we stop, the bomb blows!
     * 在记忆中，我们一路走来，停在那里，炸弹就爆炸了！
     * */
    input = read_line();
    phase_5(input);
    phase_defused();
    printf("Good work!  On to the next...\n");

    /* This phase will never be used, since no one will get past the
     * earlier ones.  But just in case, make this one extra hard.
     * 这个阶段永远不会被使用，因为没有人能通过
    以前的那些。但以防万一，把这件事做得更难。
     * */
    input = read_line();
    phase_6(input);
    phase_defused();

    /* Wow, they got it!  But isn't something... missing?  Perhaps
     * something they overlooked?  Mua ha ha ha ha!
     *哇，他们找到了！但不是什么。。。失踪了？也许他们忽略了什么？哈哈哈哈哈！
     * */
    
    return 0;
}
