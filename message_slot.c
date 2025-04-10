
#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#include "message_slot.h"
#include <linux/module.h>
#include <linux/kernel.h>   
#include <linux/string.h> 
#include <linux/fs.h>       
#include <linux/uaccess.h> 
#include <linux/slab.h>   
#include <linux/uaccess.h>  

MODULE_LICENSE("GPL");
#define max_message_channels  (1<<20)


//==============================================

typedef struct channel_s {
  char message[BUFFER_SIZE];
  unsigned int channel_id;
  unsigned int message_length;
} channel_t;

typedef struct node_channel_s {
  channel_t channel;
  struct node_channel_s* next;
} chnl_lst;

static chnl_lst* msgslot_files[256]; // used chardev so we can assume it will be 256 at most


static chnl_lst* create_node(unsigned int id) {
  chnl_lst* node = (chnl_lst*)kmalloc(sizeof(chnl_lst), GFP_KERNEL);
  if (NULL == node) return NULL;
  node->channel.channel_id = id;
  node->channel.message_length = 0;
  node->next = NULL;
  return node;
}

static chnl_lst* get_node(chnl_lst* root, unsigned int channel_id) {
  if (NULL == root) {
      return create_node(channel_id);
  }
  while (NULL != root->next && root->channel.channel_id != channel_id) {
    root = root->next;
  }
  if (root->channel.channel_id == channel_id) {
    return root;
  }
  root->next = create_node(channel_id);
  return root->next;
}


static void init_msgslot_files(void) {
  int i;
  for (i = 0; i < 256; i++) {
    msgslot_files[i] = create_node(0);
  }
}

static int device_open( struct inode* inode, struct file*  file )
{
  int minor_num = iminor(inode);
  file->private_data = msgslot_files[minor_num];
  return 0;
}


static ssize_t device_read( struct file* file,char __user* buffer,size_t length,loff_t* offset )
{
  int msg_len;
  chnl_lst* channel = (chnl_lst*)file->private_data;
  if (channel->channel.channel_id == 0) {
    printk("No channel has been set\n");
    return -EINVAL;
  }
  msg_len = channel->channel.message_length;
  // Checking if the relevant buffer has a message
  if (msg_len == 0) {
    printk("No message in the buffer\n");
    return -EWOULDBLOCK;
  }

  // Validating that the buffer is long enough for the message.
  if (length < msg_len) {
    printk("Buffer too short\n");
    return -ENOSPC;
  }
  // Checking buffer validity.
  if(!buffer) {
    printk("Invalid buffer\n");
    return -EINVAL;
  }
  if ( !access_ok( buffer, length ) ) {
    printk("Invalid buffer access\n");
    return -EFAULT;
  }
  if ( copy_to_user(buffer, channel->channel.message, msg_len) != 0 ) {
    printk("Failed to copy message to user\n");
    return -EINVAL;
  }

  return msg_len;
}

//---------------------------------------------------------------
static ssize_t device_write( struct file* file,const char __user* buffer,size_t length,loff_t* offset)
{
  chnl_lst* channel = (chnl_lst*)file->private_data;

    if ( length == 0 || length > BUFFER_SIZE) {
    printk("Invalid message length\n");
    return -EMSGSIZE;
  }

  // Checking if a channel has been set
  if (channel->channel.channel_id == 0) {
    printk("No channel has been set\n");
    return -EINVAL;
  }
  if (!buffer) {
    printk("Invalid buffer\n");
    return -EINVAL;
  }
  if (!access_ok(buffer, length)) {
    printk("can't acces buffer \n");
    return -EFAULT;
  }
  if (copy_from_user(channel->channel.message, buffer, length) != 0) {
    printk("Failed coppying message to user \n");
    return -EINVAL;
  }
  channel->channel.message_length = length;
  return length;
}

//----------------------------------------------------------------

// not sure if i need to implement this function??? put it here just in case
static int device_release( struct inode* inode, struct file* file)
{
  return 0;
}
static long device_ioctl( struct file* file,unsigned int cmd,unsigned long ioctl_param )
{
  int minor_num;
  chnl_lst* channel;
  minor_num = iminor(file->f_inode);
  // Switch according to the ioctl called
  if (ioctl_param == 0) {
    printk("Invalid channel\n");
    return -EINVAL;
  }
  if(MSG_SLOT_CHANNEL != cmd ) {
    printk("Invalid ioctl command\n");
    return -EINVAL;
  }
  channel = get_node(msgslot_files[minor_num], ioctl_param);
  // if channel is NULL, allocation failed
  if(channel == NULL) {
    printk("Failed to allocate channel\n");
    return -EINVAL;
  }
  file->private_data = channel;
  return 0;
}

//==================== =============================

struct file_operations Fops = {
  .owner	  = THIS_MODULE, 
  .read           = device_read,
  .write          = device_write,
  .open           = device_open,
  .release        = device_release,
  .unlocked_ioctl = device_ioctl,
};

//---------------------------------------------------------------
static int __init mod_init(void)
{
  int reg = register_chrdev( MAJOR_NUM, DEVICE_FILE_NAME, &Fops );
  init_msgslot_files();
  // Negative values signify an error
  if( reg < 0 ) {
    printk( KERN_ALERT " registraion failed \n");
    return -1;
  }

  return 0;
}

//---------------------------------------------------------------

static void __exit mod_cleanup(void)
{
  // Unregister the device
  int index;
  for (index = 0; index < 256; index++) {
    chnl_lst* cur_channel_node = msgslot_files[index];
    chnl_lst* temp_node;
    while (cur_channel_node != NULL) {
    temp_node = cur_channel_node;
    cur_channel_node = cur_channel_node->next;
    kfree(temp_node);
  }
  }
  unregister_chrdev(MAJOR_NUM, DEVICE_FILE_NAME);
}

module_init(mod_init);
module_exit(mod_cleanup);

