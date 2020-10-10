#include <asm/segment.h>
#include <asm/system.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <stdarg.h>
#include <errno.h>

#define MESSAGE_LENGTH 1000

extern int vsprintf(char *buf, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);
static int count_ones(char b);

struct proc_info_list {
  struct proc_info *infos[NR_PROC_INFOS];
  unsigned char lock;
  struct task_struct *q;
} proc = {{NULL, }, 1, NULL};

static void lock_proc() {
  sti(); 
  while (!proc.lock) {
    sleep_on(&proc.q);
  }
  proc.lock = 0;
  cli();
}

void unlock_proc() {
  sti();
  if (proc.lock == 1) {
    cli();
    panic(
        "[unlock_proc]: trying to unlock while proc is already "
        "unlocked.");
  }
  proc.lock = 1;
  if (proc.q != NULL) {
    wake_up(&proc.q);
  }
  cli();
}

int proc_read(
    struct m_inode * inode, struct file * filp, char * buf, int count) {
  struct proc_info *info;
  unsigned short type;
  int i;

  type = inode->i_zone[0];
  if (type >= NR_PROC_INFOS) {
    return -ERANGE;
  }
  if ((info=proc.infos[filp->f_ext[0]]) == NULL) {
    return -ENOENT;
  }
   
  if (filp->f_pos+count > info->length) {
    count = info->length - filp->f_pos;
    if (count <= 0) {
      return 0;
    }
    for (i = 0; i < count; i++) {
      put_fs_byte(info->content[filp->f_pos+i], &buf[i]);
    }
  } 
  filp->f_pos += count;
  return count;
}

/* TODO(neolicad): solve concurrency. */
void proc_write(struct m_inode * inode, struct file *filp) {
  unsigned char type;
  struct proc_info *info;
  char *content;
  struct buffer_head *bh;
  char *b_data;
  int used;
  int length, total_length=0;
  int i,j;
  int empty_slot;

  type = inode->i_zone[0];
  if (type >= NR_PROC_INFOS) {
    panic("proc_write: inode type out of range!");
  }
  lock_proc();
  for (empty_slot = 0; i < NR_PROC_INFOS; empty_slot++) {
    if (proc.infos[i] == NULL) {
      break;
    }
  }
  if (empty_slot == NR_PROC_INFOS) {
    panic("[proc_write] No empty entries for process information!");
  }
  filp->f_ext[0] = empty_slot;
  info = (struct proc_info *) malloc(sizeof(struct proc_info));
  content = (char *) malloc(sizeof(char) * MESSAGE_LENGTH);
  info->content = content;
  if (type == 0) {
    /* psinfo */
    /* TODO(neolicad): make sure the total length does not exceed 
     * MESSAGE_LENGTH */
    if((length=sprintf(content, "pid\tstate\tfather\tcounter\tstart_time\n")) 
        < 0) {
      panic("proc_write: failed to write content into buffer!");
    }
    content += length;
    total_length += length;
    for (i = 0; i < NR_TASKS; i++) {
      if (task[i] == NULL) {
        continue;
      }
      if((length=
              sprintf(
                  content, 
                  "%ld\t%ld\t%ld\t%ld\t%ld\n",
                  task[i]->pid,
                  task[i]->state,
                  task[i]->father,
                  task[i]->counter,
                  task[i]->start_time)) 
          < 0) {
        panic("proc_write: failed to write content into buffer!");
      } 
      content += length;
      total_length += length;
    }
    info->length = total_length;
  } else if (type == 1) {
    struct super_block *super;
    /* hdinfo */
    /* TODO: clean up super block? */
    if (!(super=get_super(inode->i_dev))) {
      panic("[proc_write] Failed to read super_block!");
    }
    if ((length=sprintf(content, "inodes #: %d\n", super->s_ninodes)) < 0) {
      panic("[proc_write] failed to write to proc info content!");
    }
    content += length;
    total_length += length;
    if ((length=sprintf(content, "data blocks #: %d\n", super->s_nzones)) < 0) {
      panic("[proc_write] failed to write to proc info content!");
    }
    content += length;
    total_length += length;
    if ((length=sprintf(
            content,
            "blocks taken by imap #: %d\n", 
            super->s_imap_blocks)) 
        < 0) {
      panic("[proc_write] failed to write to proc info content!");
    }
    content += length;
    total_length += length;
    if ((length=sprintf(
            content,
            "blocks taken by zmap #: %d\n", 
            super->s_zmap_blocks)) 
        < 0) {
      panic("[proc_write] failed to write to proc info content!");
    }
    content += length;
    total_length += length;
    used = 0;
    for (i = 0; i < 8; i++) {
      if(!(bh=super->s_imap[i])) {
        break;
      } 
      if (!(b_data=bh->b_data)) {
        panic("data pointer in buffer_head is NULL!");
      }
      for (j = 0; j < 1024; j++) {
        used += count_ones(b_data[j]);
      }
    }
    if ((length=sprintf(
            content,
            "inode used : %d/%d\n", 
            used,
            super->s_ninodes)) 
        < 0) {
      panic("[proc_write] failed to write to proc info content!");
    }
    content += length;
    total_length += length;
    used = 0;
    for (i = 0; i < 8; i++) {
      if(!(bh=super->s_zmap[i])) {
        break;
      } 
      if (!(b_data=bh->b_data)) {
        panic("data pointer in buffer_head is NULL!");
      }
      for (j = 0; j < 1024; j++) {
        used += count_ones(b_data[j]);
      }
    }
    if ((length=sprintf(
            content,
            "blocks used : %d/%d\n", 
            used,
            super->s_nzones)) 
        < 0) {
      panic("[proc_write] failed to write to proc info content!");
    }
    content += length;
    total_length += length;
    info->length = total_length;
  }
  proc.infos[empty_slot] = info;
  unlock_proc();
}

void proc_clear(struct m_inode *inode, struct file *filp) {
  unsigned char type;
  struct proc_info *info;

  type = inode->i_zone[0];
  if (type >= NR_PROC_INFOS) {
    panic("proc_clear: inode type out of range!");
  }
  lock_proc();
  if ((info=proc.infos[filp->f_ext[0]]) == NULL) {
    panic("proc_clear: trying to clear an already-cleared entry!");
  }
  free(info->content);
  free(info);
  proc.infos[filp->f_ext[0]] = NULL;
  unlock_proc();
}

int sprintf(char *buf, const char *fmt, ...) {
  int length;
  va_list va;

  va_start(va, fmt);
  length = vsprintf(buf, fmt, va);
  va_end(va);
  return length;
}

static int count_ones(char b) {
  int i;
  int count = 0;
  for (i = 0; i < 8; i++) {
    count += (b & 0x1);
    b >>= 1;
  }
  return count;
}
