#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>

#include "simple.h"

static int simplefs_readdir(struct file *filp, void *dirent, filldir_t filldir)
{
    //ls will list nothing as of now.
    printk(KERN_INFO "I'm reading dir now\n");
    return 0;
}

const struct file_operations simplefs_dir_operations = {
    .owner = THIS_MODULE,
    .readdir = simplefs_readdir,
};

struct dentry *simplefs_lookup(struct inode *parent_inode,
                    struct dentry *child_dentry, unsigned int flags)
{
    // lookup is used for dentry association
    printk(KERN_INFO "I'm lookup dentry now\n");
    return NULL;
}

static struct inode_operations simplefs_inode_ops = {
    .lookup = simplefs_lookup,
};

struct inode * simplefs_get_inode(struct super_block *sb,
                    const struct inode *dir, umode_t mode, dev_t dev)
{
    struct inode *inode = new_inode(sb);

    if (inode) {
        inode->i_ino = get_next_ino();
        inode_init_owner(inode, dir, mode);

        inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;

        switch (mode & S_IFMT) {
        case S_IFDIR:
            /* i_nlink will be initialized to 1 by default, we inc to 2
             * for directories, for '.' entry
             */
            inc_nlink(inode);
            break;
        case S_IFREG:
        case S_IFLNK:
        default:
            printk(KERN_ERR "simplefs can create meaningfull inode for root dir only\n");
            return NULL;
            break;
        }
    }

    return inode;
}

int simplefs_fill_super(struct super_block *sb, void *data, int silent)
{
    struct inode *inode;

    sb->s_magic = SIMPLEFS_MAGIC;
    inode = simplefs_get_inode(sb, NULL, S_IFDIR, 0); //2->3
    inode->i_op = &simplefs_inode_ops;
    inode->i_fop = &simplefs_dir_operations; //2->3
    sb->s_root = d_make_root(inode);
    if (!sb->s_root)
        return -ENOMEM;

    return 0;
}

static void simplefs_kill_superblock(struct super_block *s)
{
    printk(KERN_INFO "simplefs superblock is destroyed, Unmount succesfully\n");
    return;
}

static struct dentry *simplefs_mount(struct file_system_type *fs_type,
                        int flags, const char * dev_name, void *data)
{
    struct dentry *ret;
    
    ret = mount_bdev(fs_type, flags, dev_name, data, simplefs_fill_super); //1->2

    if (unlikely(IS_ERR(ret)))
        printk(KERN_ERR "Error mounting simplefs");
    else
        printk(KERN_INFO "Simple fs is mounted succesfully on [%s]\n", dev_name);

    return ret;
}

struct file_system_type simplefs_fs_type = {
    .owner = THIS_MODULE,
    .name = "simplefs",
    .mount = simplefs_mount,
    .kill_sb = simplefs_kill_superblock,
};

static int __init simplefs_init(void)
{
    int ret;
    printk(KERN_ALERT "hello world");

    ret = register_filesystem(&simplefs_fs_type); //0->1
    if (likely(ret == 0))
        printk(KERN_INFO "Sucessfully register simplefs\n");
    else
        printk(KERN_ERR "Failed to register simplefs, Error: [%d]\n", ret);


    return ret;
}

static void __exit simplefs_exit(void)
{
    int ret;
    printk(KERN_ALERT "good bye");

    ret = unregister_filesystem(&simplefs_fs_type);
    if (likely(ret == 0))
        printk(KERN_INFO "Sucessfully unregister simplefs\n");
    else
        printk(KERN_ERR "Failed to unregister simplefs, Error: [%d]\n", ret);
}

module_init(simplefs_init);
module_exit(simplefs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rock");
