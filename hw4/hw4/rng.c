// rng.c !!

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/random.h>
#include <errno.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <asm/semaphore.h>
#include "rng.h"

#define MODULE_NAME "rng"
#define DEFAULT_MAX_GUESSES 3
#define DEFAULT_LEVEL 0
MODULE_LICENSE("GPL");

//function definitions
int my_open(struct inode *, struct file *);
int my_release(struct inode *, struct file *);
ssize_t my_read(struct file *, char *, size_t, loff_t *);
ssize_t my_write(struct file *, const char *, size_t, loff_t *);
int my_ioctl(struct inode *, struct file *,unsigned int, unsigned long);
loff_t my_llseek(struct file*,loff_t,int);

int alloc_init_device_data(void ** devData);

int gen_rand(int max);

struct file_operations my_fops=
{
	.open = my_open,
	.release = my_release,
	.read = my_read,
	.write = my_write,
	.llseek= my_llseek,
	.ioctl = my_ioctl,
};

typedef struct game
{
	struct semaphore sem;
	int nr_guesses;
	int max_guesses;
	int level;
	int rand_val;
	int minor;
	int count;
} Game;

//GLOBAL DEFS
int major_number = 0;
int nr_games = 0;
MODULE_PARM(nr_games, "i" );
struct semaphore main_sem;
Game* games_collection;


//HELP FUNCTIONS
int gen_rand(int max)
{
	unsigned int val;
	get_random_bytes(&val,sizeof(unsigned int));
	return val % max;
}

void make_new_game(Game* game)
{
	game->nr_guesses=0;
	game->rand_val=gen_rand((game->level+1)*10);
}

// MODULE FUNCTIONS
int init_module(void)
{
    int major = 0;
    major = register_chrdev(major_number,MODULE_NAME,&my_fops);

    if (major < 0)
    {
    	return major;
    }

    major_number = major;

	games_collection=kmalloc(nr_games*sizeof(Game),GFP_KERNEL);

	if (games_collection==NULL)
	{
		printk("Alloc error\n");
		return -ENOSPC;
	}

	sema_init(&main_sem,1);

	int i;
	for (i=0;i<nr_games;i++)
	{
		games_collection[i].minor=-1;
		games_collection[i].count=0;
		sema_init(&games_collection[i].sem,1);
	}

    return 0;
}

void cleanup_module(void)
{
	major_number = 0;
	kfree(games_collection);
	unregister_chrdev(major_number,MODULE_NAME);
}


int my_open(struct inode *inode, struct file * filp){

	down_interruptible(&main_sem);

	int i,minor=MINOR(inode->i_rdev),currIndex=-1;
	for (i=0;i<nr_games;i++)
	{
		if (games_collection[i].minor==minor)
		{
			if ((filp->private_data!=NULL) &&
					*(int*)filp->private_data==currIndex)
			{
				up(&main_sem);
				return 0;
			}
			filp->private_data=(void*)kmalloc(sizeof(int),GFP_KERNEL);
			*((int*)filp->private_data)=i; // ZZZ
			games_collection[i].count++;
			up(&main_sem);
			return 0;
		}
		if (games_collection[i].minor==-1 && currIndex==-1)
		{
			currIndex=i;
		}

	}
	if (currIndex==-1)
	{
		up(&main_sem);
		return -ENOSPC;
	}

	filp->private_data=(void*)kmalloc(sizeof(int),GFP_KERNEL);

	*((int*)filp->private_data)=currIndex; //ZZZ
	games_collection[currIndex].minor=minor;
	games_collection[currIndex].count++;
	games_collection[currIndex].level=DEFAULT_LEVEL;
	games_collection[currIndex].max_guesses=DEFAULT_MAX_GUESSES;
	make_new_game(&games_collection[currIndex]);
	up(&main_sem);
	return 0;

}

int my_release(struct inode * currInode, struct file *filp)
{
	int currIndex=*(int*)filp->private_data;
	if (currIndex==-1)
	{
		return -EPERM;
	}

	down_interruptible(&main_sem);

	kfree(filp->private_data);
	filp->private_data=NULL;
	games_collection[currIndex].count--;
	if (games_collection[currIndex].count==0)
	{
		games_collection[currIndex].minor=-1;
	}

	up(&main_sem);

	return 0;
}

ssize_t my_read(struct file *filp, char* buf, size_t count, loff_t * offp){

	if (buf==NULL)
	{
		return -EFAULT;
	}

	int temp,currIndex=-1;
	if(filp->private_data != NULL)
	{
		currIndex=*((int*)filp->private_data);
	}
	else
	{
		return -EPERM;
	}

	down_interruptible(&main_sem);
	temp=games_collection[currIndex].rand_val;
	make_new_game(&games_collection[currIndex]);
	up(&main_sem);

	if(copy_to_user(buf,&temp,sizeof(char))!=0)
	{
		return -ENOSPC;
	}
	return 1;
}

ssize_t my_write(struct file *filp, const char* buf, size_t count, loff_t * offp)
{
	int currIndex=*((int*)filp->private_data),out;
	char guess;
	if (currIndex==-1)
	{
		return -EPERM;
	}

	down_interruptible(&main_sem);

	if (copy_from_user(&guess,buf,sizeof(char)) !=0 )
	{
		up(&main_sem);
		return -EFAULT;
	}

	if (guess==games_collection[currIndex].rand_val)
	{
		make_new_game(&games_collection[currIndex]);
		out=games_collection[currIndex].max_guesses;
		up(&main_sem);
		return out;
	}
	games_collection[currIndex].nr_guesses++;
	if (games_collection[currIndex].nr_guesses>=
			games_collection[currIndex].max_guesses)
	{
		make_new_game(&games_collection[currIndex]);
	}
	out=games_collection[currIndex].max_guesses-
		games_collection[currIndex].nr_guesses;

	up(&main_sem);
	return out;
}

int m_llseek(struct file *filp,loff_t f_pos,int i)
{
	return -ENOSYS;
}

int my_ioctl(struct inode *currInode, struct file* filp,
		unsigned int cmd, unsigned long arg)
{
	int currIndex=*((int*)filp->private_data);
	int currGuess;

	if (currIndex==-1)
	{
		return -EPERM;
	}

	down_interruptible(&main_sem);

	switch (cmd)
	{
		case RNG_LEVEL:
			if (arg < 0 || arg > 2)
			{
				up(&main_sem);
				return -EINVAL;
			}
			games_collection[currIndex].level=arg;
			make_new_game(&games_collection[currIndex]);
			break;

		case RNG_GUESS:
			if(arg<=0 || arg > INT_MAX)
			{
				up(&main_sem);
				return -EINVAL;
			}
			games_collection[currIndex].max_guesses=arg;
			make_new_game(&games_collection[currIndex]);
			break;

		case RNG_HINT:
			currGuess=gen_rand(6*games_collection[currIndex].level+7)+
				games_collection[currIndex].rand_val-3*
				(games_collection[currIndex].level+1);
			if (currGuess<0)
			{
				currGuess=0;
			}

			games_collection[currIndex].nr_guesses++;
			if(games_collection[currIndex].nr_guesses>=games_collection[currIndex].max_guesses)
			{
				make_new_game(&games_collection[currIndex]);
			}

			up(&main_sem);

			return currGuess;
			break;

		default:
			up(&main_sem);
			return -ENOTTY;
			break;
	}
	up(&main_sem);
	return 0;
}

loff_t my_llseek(struct file* filp,loff_t offp,int number)
{
	return -ENOSYS;
}






