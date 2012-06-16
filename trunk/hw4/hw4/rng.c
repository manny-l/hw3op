#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/random.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <asm/semaphore.h>
#include "rng.h"

/*------------------------------------------
Definitions & Global parameters
------------------------------------------*/

#define MODULE_NAME "rng"
MODULE_LICENSE("GPL");

//global parameters
int major_number = 0;
int nr_games = 0;
MODULE_PARM(nr_games, "i" );

//function definitions
int my_open(struct inode *, struct file *);
int my_release(struct inode *, struct file *);
ssize_t my_read(struct file *, char *, size_t, loff_t *);
ssize_t my_write(struct file *, const char *, size_t, loff_t *);
int my_ioctl(struct inode *, struct file *,unsigned int, unsigned long);
loff_t my_llseek(struct file*,loff_t,int);

int alloc_init_device_data(void ** devData);

int getRandomForMode(int mode);
int getHint(int rnd_val,int level);
int my_abs(int x);

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
} * device_data;


/*------------------------------------------
			Module functions
------------------------------------------*/

int init_module(void)
{
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

	return 0;
}

void cleanup_module(void)
{
	//unregistering the module
	unregister_chrdev(major_number,MODULE_NAME);
	major_number = 0;
}


/*------------------------------------------
			Devices functions
------------------------------------------*/

int my_open(struct inode * currInode, struct file * filp){

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
}

int my_release(struct inode * currInode, struct file *filp)
{
	//check parameters
	if (currInode==NULL || filp==NULL)
	{
		return -EFAULT;
	}

	//release allocated data
	if (filp->private_data)
	{
		kfree(filp->private_data);
		filp->private_data=NULL;
	}

	return 0;
}

ssize_t my_read(struct file *filp, char* buff, size_t count, loff_t * offp){

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

	//write to buff & update cur_val

	for(ind=0; ind<count; ind++)
	{
		if(copy_to_user(&buff[ind],&(currData->rnd_val),sizeof(char)) > 0 )
		{
			return -EFAULT;
		}

		currData->nr_guesses = 0;
		currData->rnd_val = getRandomForMode(currData->level);

		/*
		if(currData->sType == ARITH_MODE)
		{
			currData->cur_val = (currData->cur_val+currData->d)%256;
		}
		else
		{//Geometric type
			currData->cur_val = (currData->cur_val*currData->q)%256;
		}
		*/
	}

	return ind;
}

ssize_t my_write(struct file *filp, const char* buff, size_t count, loff_t * offp){

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
	return currData->max_guesses - currData->nr_guesses;

	/*
	if(copy_from_user(&(currData->cur_val),buff,count) > 0)
	{
		return -EFAULT;
	}

	//mod 256 to the new cur_val
	currData->cur_val = currData->cur_val%256;

	//update last write val
	currData->last_write_val = currData->cur_val;


	return count;
	*/
}

int my_ioctl(struct inode *currInode, struct file* filp,unsigned int cmd, unsigned long arg){

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



	/*
	switch(cmd){
		//reset cur_val
		case SEQ_RESET:
			pCurrData->cur_val = pCurrData->last_write_val;
			break;
		//switch ARITH_MODE and GEOM_MODE
		case SEQ_SWCH_MOD:
			if (pCurrData->sType == ARITH_MODE){
				pCurrData->sType = GEOM_MODE;
			}
			else{
				pCurrData->sType = ARITH_MODE;
			}
			break;
		//chande to ARITH_MODE/GEOM_MODE
		case SEQ_CHG_MOD:
			if (arg==ARITH_MODE){
				pCurrData->sType=ARITH_MODE;
			}
			else if (arg==GEOM_MODE){
				pCurrData->sType=GEOM_MODE;
			}
			else {
				return -EFAULT;
			}
			break;
		//change device's d parameter
		case SEQ_CHG_D:
			if (arg > 255||arg < 0) {
				return -EFAULT;
			}
			pCurrData->d = arg;
			break;
		//get device's d parameter
		case SEQ_GET_D:
			res = pCurrData->d;
			break;
		//change device's q parameter
		case SEQ_CHG_Q:
			if (arg > 255 || arg < 0) {
				return -EFAULT;
			}
			pCurrData->q = arg;
			break;
		//get device's q parameter
		case SEQ_GET_Q:
			res = pCurrData->q;
			break;
		//if all else fail => error
		default:
			res = -EFAULT;
	}
	*/

	return res;
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

	return 0;
}

int getHint(int rnd_val,int level)
{
	int x = 10000;

	while (!(my_abs(rnd_val-x) < 3*(level +1) ))
	{
		get_random_bytes(&x, sizeof(char));
	}

	return x;
}


int my_abs(x)
{
	return (x>=0 ? x : -x);
}

int getRandomForMode(int mode)
{
	int tmpInt;

	switch (mode)
	{
		case 0:
			get_random_bytes(&tmpInt, sizeof(char));
			tmpInt = tmpInt % 100;
			break;

		case 1:
			tmpInt = 100;
			while (tmpInt > 19)
			{
				get_random_bytes(&tmpInt, sizeof(char));
				tmpInt = tmpInt % 10;
			}
			break;

		case 2:
			tmpInt = 100;
			while (tmpInt > 29)
			{
				get_random_bytes(&tmpInt, sizeof(char));
				tmpInt = tmpInt % 10;
			}
			break;
	}

	return tmpInt;
}

/*
int main()
{
	return 0;
}
*/

