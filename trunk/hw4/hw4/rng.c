#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/random.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <asm/semaphore.h>
//#include <pthread.h>
#include "rng.h"

#define MODULE_NAME "rng"
#define DEFAULT_MAX_GUESSES 3
#define DEFAULT_LEVEL 0

/*------------------------------------------
Definitions & Global parameters
------------------------------------------*/

#define MODULE_NAME "rng"
MODULE_LICENSE("GPL");

//global parameters
int major_number = 0;
int nr_games = 0;
//int nr_counter = 0;
//struct semaphore gsem;
MODULE_PARM(nr_games, "i" );

//function definitions
int my_open(struct inode *, struct file *);
int my_release(struct inode *, struct file *);
ssize_t my_read(struct file *, char *, size_t, loff_t *);
ssize_t my_write(struct file *, const char *, size_t, loff_t *);
int my_ioctl(struct inode *, struct file *,unsigned int, unsigned long);
loff_t my_llseek(struct file*,loff_t,int);

int alloc_init_device_data(void ** devData);

int generate_random_val(int max);
int getRandomForMode(int mode);
int getHint(int rnd_val,int level);
//int my_abs(int x);
int getRandomInRange(int from,int to);

//file operations definition
struct file_operations my_fops={
	.open = my_open,
	.release = my_release,
	.read = my_read,
	.write = my_write,
	.llseek= my_llseek,
	.ioctl = my_ioctl,
};

//device data struct definition
typedef struct device_data_s
{
	int rnd_val;
	int level;
	int nr_guesses;
	int max_guesses;
	struct semaphore sem;
} * device_data;

// TALI
typedef struct game {//holds private game data - fill later
	int nr_guesses;
	int max_guesses;
	int level;
	char random_val;
	int minor;
	struct semaphore sem;
	int count;
} Game;

Game* games;
int next_slot=0;
void new_game(Game* game);


/*------------------------------------------
			Module functions
------------------------------------------*/

int init_module(void)
{
	if(nr_games<=0) return -EINVAL;

	//registering the module
	int major = 0;
	major = register_chrdev(major_number,MODULE_NAME,&my_fops);

	//checking for register errors
	if (major < 0)
	{
		return major;
	}

	//saving the major_number
	major_number = major;


	games=kmalloc(nr_games*sizeof(Game),GFP_KERNEL);
	if(games==NULL)
	{
		//printk("Error allocating games\n");
		return -ENOSPC;
	}
	int i;
	for (i=0;i<nr_games;i++)
	{
		games[i].minor=-1;
		games[i].count=0;
		sema_init(&games[i].sem,1);
	}

	return 0;
}

void cleanup_module(void)
{
	//unregistering the module
	kfree(games);
	unregister_chrdev(major_number,MODULE_NAME);
	major_number = 0;
}


/*------------------------------------------
			Devices functions
------------------------------------------*/

int my_open(struct inode *inode, struct file * filp){

	int i,minor=MINOR(inode->i_rdev),slot=-1;
	for (i=0;i<nr_games;i++) {
		down_interruptible(&games[i].sem);
		if (games[i].minor==minor) {
			if(slot!=-1) up(&games[slot].sem);
			if(filp->private_data!=NULL && *(int*)filp->private_data==slot){
				up(&games[i].sem);
				return 0;
			}
			filp->private_data=(void*)kmalloc(sizeof(int),GFP_KERNEL);
			*((int*)filp->private_data)=slot;
			games[i].count++;
			up(&games[i].sem);
			return 0;
		}
		if (games[i].minor==-1 && slot==-1) slot=i;
		if (slot!=i) up(&games[i].sem);
	}
	if (slot==-1) {
		return -ENOSPC;
	}
	filp->private_data=(void*)kmalloc(sizeof(int),GFP_KERNEL);
	*((int*)filp->private_data)=slot;
	games[slot].count++;
	games[slot].minor=minor;
	games[slot].max_guesses=DEFAULT_MAX_GUESSES;
	games[slot].level=DEFAULT_LEVEL;
	new_game(&games[slot]);
	up(&games[slot].sem);
	return 0;


/*
	//local parameters
	int minor;

	//check parameters
	if (currInode==NULL || filp==NULL)
	{
		return -EFAULT;
	}

	//validating the MINOR
	minor = MINOR(currInode->i_rdev);
	if (minor < 0 || minor > nr_games)
	{
		return -ENODEV;
	}

	//allocting and init the device data
	if(alloc_init_device_data(&(filp->private_data)) < 0)
	{
		return -EFAULT;
	}


	device_data myGame = (device_data)filp->private_data;

	myGame->level=0;
	myGame->max_guesses=3;
	myGame->rnd_val=getRandomForMode(0);

	return 0;
*/
}

int my_release(struct inode * currInode, struct file *filp)
{
	int slot=*(int*)filp->private_data;
	if (slot==-1) return -EPERM; //ERROR
	down_interruptible(&games[slot].sem);
	kfree(filp->private_data);
	filp->private_data=NULL;
	games[slot].count--;
	if(games[slot].count==0)
	{
		games[slot].minor=-1;
	}
	up(&games[slot].sem);
	return 0;

	/*
	//check parameters
	if (currInode==NULL || filp==NULL)
	{
		return -EFAULT;
	}

	//release allocated data
	device_data myGame = (device_data)filp->private_data;
	if (myGame)
	{
		kfree(myGame);
		filp->private_data=NULL;
	}

	return 0;
	*/
}

ssize_t my_read(struct file *filp, char* buf, size_t count, loff_t * offp){

	/*
	//local parameters
	int ind;
	device_data currData;

	//validate params
	if (filp==NULL || filp->private_data == NULL || buff==NULL || offp==NULL || count < 0 )
	{
		return -EFAULT;
	}

	//check if file is open for read
	if(!(filp->f_mode&FMODE_READ))
	{
		return -EACCES; //file atribute conflict
	}

	//casting the data pointer
	currData = (device_data)(filp->private_data);

	down_interruptible(&(currData->sem));

	//write to buff & update cur_val

	for(ind=0; ind<count; ind++)
	{
		currData->nr_guesses = 0;
		currData->rnd_val = getRandomForMode(currData->level);

		if(copy_to_user(&buff[ind],&(currData->rnd_val),sizeof(char)) > 0 )
		{
			return -EFAULT;
		}
	}
	up(&(currData->sem));
	return ind;
	*/


	if(buf==NULL)
	{
		//return -EINVAL;
		return -ENOSPC;
	}
	int i,temp,slot=-1;
	if(filp->private_data){
		slot=*((int*)filp->private_data);
	}
	else { //
		return -EPERM; //ERROR
	}
	down_interruptible(&games[slot].sem);
	temp=games[slot].random_val;
	//new game:
	new_game(&games[slot]);
	up(&games[slot].sem);
	//copy result


	if(copy_to_user(buf,&temp,sizeof(char))!=0) return -ENOSPC;


	return 0;

	//return temp;




}

ssize_t my_write(struct file *filp, const char* buf, size_t count, loff_t * offp){

	int i,slot=*((int*)filp->private_data),out;
	char guess;
	if (slot==-1) return -EPERM; //ERROR
	down_interruptible(&games[slot].sem);
	if (copy_from_user(&guess,buf,sizeof(char))!=0) {
		up(&games[slot].sem);
		return -ENOSPC;
	}
	if (guess==games[slot].random_val) {
		new_game(&games[slot]);
		out=games[slot].max_guesses;
		up(&games[slot].sem);
		return out;//what to return if game won?
	}
	games[slot].nr_guesses++;
	if (games[slot].nr_guesses>=games[slot].max_guesses) {
		new_game(&games[slot]);
	}
	out=games[slot].max_guesses-games[slot].nr_guesses;
	up(&games[slot].sem);
	return out;

	/*
	//local parameters
	device_data currData;

	//validate params
	if (filp==NULL || filp->private_data == NULL
		|| buff==NULL || offp==NULL)
	{
		return -EFAULT;
	}

	//check buffer size
	if(count != 4)
	{ //TODO
		return -EINVAL;
	}

	//check if file is open for writing
	if(!(filp->f_mode&FMODE_WRITE))
	{
		return -EACCES; //file attribute conflict
	}

	//casting the data pointer
	currData = (device_data)(filp->private_data);

	down_interruptible(&(currData->sem));

	//update cur_val

	int user_val = 0;

	if(copy_from_user(&(user_val),buff,count) > 0)
	{
		return -EFAULT;
	}

	if (user_val == currData->rnd_val)
	{
		currData->nr_guesses=0;
		currData->rnd_val = getRandomForMode(currData->level);
		return currData->max_guesses;
	}

	//WRONG GUESS
	//reached max guesses
	if (currData->nr_guesses + 1 == currData->max_guesses)
	{
		currData->nr_guesses=0;
		currData->rnd_val = getRandomForMode(currData->level);
		return currData->max_guesses;
	}

	//has more guesses
	currData->nr_guesses++;

	int rslt = currData->max_guesses - currData->nr_guesses;

	up(&(currData->sem));

	return rslt;
	*/
}

int my_ioctl(struct inode *currInode, struct file* filp,unsigned int cmd, unsigned long arg){

	int i,x,slot=*((int*)filp->private_data);
	if (slot==-1) return -EPERM; //ERROR
	down_interruptible(&games[slot].sem);
	switch (cmd) {
		case RNG_LEVEL:
			if (arg<0 || arg>2) {
				up(&games[slot].sem);
				return -EINVAL; //ERROR
			}
			games[slot].level=arg;
			new_game(&games[slot]);
			break;
		case RNG_GUESS:
			if(arg<=0) {
				up(&games[slot].sem);
				return -EINVAL;
			}
			games[slot].max_guesses=arg;
			new_game(&games[slot]);
			break;
		case RNG_HINT:
			x=generate_random_val(6*games[slot].level+7)+games[slot].random_val-3*(games[slot].level+1);
			games[slot].nr_guesses++;
			if(games[slot].nr_guesses>=games[slot].max_guesses) {
				new_game(&games[slot]);
			}
			up(&games[slot].sem);
			return x>0? x : 0;
			break;
		default:
			up(&games[slot].sem);
			return -EINVAL;//error
			break;
	}
	up(&games[slot].sem);
	return 0;

	/*
	//local parameters
	int res = 0;

	//validate params
	if (filp==NULL || filp->private_data == NULL || currInode==NULL)
	{
		return -EFAULT;
	}

	//casting the data pointer
	device_data pCurrData = (device_data)(filp->private_data);

	//executing the given command

	switch(cmd)
	{
		case RNG_LEVEL:

			if (arg > 2||arg < 0)
			{
				return -EFAULT;
			}

			pCurrData->level = arg;
			pCurrData->rnd_val = getRandomForMode(arg);
			pCurrData->nr_guesses= 0 ;
			break;

		case RNG_GUESS:

			if (arg > INT_MAX||arg < 0)
			{
				return -EFAULT;
			}

			pCurrData->max_guesses=arg;
			pCurrData->rnd_val = getRandomForMode(arg);
			pCurrData->nr_guesses= 0 ;

			break;

		case RNG_HINT:

			pCurrData->nr_guesses++;

			res = getHint(pCurrData->rnd_val,pCurrData->level);

			if (pCurrData->nr_guesses + 1 == pCurrData->max_guesses)
			{
				pCurrData->nr_guesses=0;
				pCurrData->rnd_val = getRandomForMode(arg);
			}

			break;

		default:
			res = -EFAULT;
	}

	return res;
	*/
}

//the module doesn't implement this function
loff_t my_llseek(struct file* filp,loff_t offp,int number)
{
	return -ENOSYS;
}


/*------------------------------------------
			help functions
------------------------------------------*/


int alloc_init_device_data(void ** devData){

	//allocate the new data struct
	(*devData)=(void *)kmalloc(sizeof(struct device_data_s),GFP_KERNEL);
	if (devData==NULL)
	{
		return -EFAULT;
	}

	//casting the data pointer
	device_data pCurrData = (device_data)(*devData);

	//initiate the data struct
	/*
	pCurrData->cur_val = 0;
	pCurrData->last_write_val=0;
	pCurrData->d = 1;
	pCurrData->q = 2;
	pCurrData->sType = ARITH_MODE;
	*/

	pCurrData->level = 0;
	pCurrData->max_guesses = 3;
	pCurrData->rnd_val = getRandomForMode(0);
	pCurrData->nr_guesses= 0 ;

	sema_init(&(pCurrData->sem),0);

	return 0;
}

int getHint(int rnd_val,int level)
{
	return getRandomInRange(rnd_val-3*(level+1),rnd_val+3*(level+1));

	/*
	int x = 10000;

	while (!(my_abs(rnd_val-x) < 3*(level +1) ))
	{
		get_random_bytes(&x, sizeof(char));
	}

	return x;
	*/
}

/*
int my_abs(x)
{
	return (x>=0 ? x : -x);
}
*/

int getRandomInRange(int from,int to)
{
	int tmpInt;
	get_random_bytes(&tmpInt, sizeof(char));
	tmpInt = tmpInt % (to+1-from);
	tmpInt += from;

	return tmpInt;
}

int getRandomForMode(int mode)
{
	int tmpInt;
	get_random_bytes(&tmpInt, sizeof(char));

	switch (mode)
	{
		case 0:
			return tmpInt % 10;
			break;

		case 1:
			return tmpInt % 20;
			break;

		case 2:
			return tmpInt % 30;
			break;
	}

	return tmpInt;
}

int generate_random_val(int max) {
	unsigned int val;
	get_random_bytes(&val,sizeof(unsigned int));
	return val % max;
}

void new_game(Game* game) {
	game->nr_guesses=0;
	game->random_val=generate_random_val((game->level+1)*10);
}

/*
int main()
{
	return 0;
}
*/

