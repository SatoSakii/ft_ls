#include "ft_ls.h"

static int	acl_errno_valid(int err)
{
	if (err == EBUSY || err == EINVAL || err == ENOENT)
		return (0);
	if (err == ENOSYS || err == ENOTSUP || err == EOPNOTSUPP)
		return (0);
	return (1);
}

#ifdef __APPLE__

// darwin (mac kernel) keeps its acl outside of the xattr
// acl_get_link_np is the only way
static int	query_acl(const char *path, t_aclinfo *ai)
{
	acl_t		acl;
	acl_entry_t	entry;
	int			has;

	ai->has_scontext = 0;
	errno = 0;
	acl = acl_get_link_np(path, ACL_TYPE_EXTENDED);
	ai->err = errno;
	if (!acl)
		return (0);
	has = (acl_get_entry(acl, ACL_FIRST_ENTRY, &entry) == 0);
	acl_free(acl);
	ai->err = 0;
	return (has);
}

#else

static char g_attr_buf[ATTR_BUF_SIZE];

static int	attr_listed(ssize_t size, const char *name)
{
	ssize_t	i;

	i = 0;
	while (i < size)
	{
		if (!strcmp(g_attr_buf + i, name))
			return (1);
		i += (ssize_t)strlen(g_attr_buf + i) + 1;
	}
	return (0);
}

// on linux, acl is a xattr, so llistxattr answers
// the posix acl names means '+', selinux label without acl means '.'
// kernel caps names list at 64Ko
static int	query_acl(const char *path, t_aclinfo *ai)
{
	ssize_t	n;

	ai->has_scontext = 0;
	errno = 0;
	n = llistxattr(path, g_attr_buf, ATTR_BUF_SIZE);
	ai->err = errno;
	if (n < 0)
		return (-1);
	ai->err = 0;
	ai->has_scontext = attr_listed(n, "security.selinux");
	if (attr_listed(n, "system.posix_acl_access") || attr_listed(n, "system.posix_acl_default"))
		return (1);
	return (0);
}

#endif

// ls remembers the first device whose acl query failed with a "not supported"
// errno, and answers no for every later file of that device: one system call
// per mount point instead of one per file
static int	acl_cached(const char *path, const t_file *f, t_aclinfo *ai)
{
	static int		known = 0;
	static dev_t	device;
	static int		answer;
	int				n;

	if (f->stat_ok && known && f->st.st_dev == device)
	{
		ai->has_scontext = 0;
		ai->err = ENOTSUP;
		return (answer);
	}
	n = query_acl(path, ai);
	if (f->stat_ok && n <= 0 && !acl_errno_valid(ai->err) && !ai->has_scontext)
	{
		known = 1;
		device = f->st.st_dev;
		answer = n;
	}
	return (n);
}

// only the long format pays for this call. a file that ls cannot reach keeps
// ACL_UNKNOWN: it prints '?' rather than pretending the acl is absent
void	acl_gobble(t_file *f, const char *path)
{
	t_aclinfo	ai;
	int			n;

	if (g_format != FMT_LONG)
		return ;
	n = acl_cached(path, f, &ai);
	if (n > 0)
		f->acl_type = ACL_YES;
	else if (ai.has_scontext)
		f->acl_type = ACL_CONTEXT;
	else if (n < 0 && (ai.err == EACCES || ai.err == ENOENT
			|| ai.err == ENOTDIR || ai.err == ELOOP))
		f->acl_type = ACL_UNKNOWN;
	else
		f->acl_type = ACL_NONE;
	if (f->acl_type != ACL_NONE)
		g_any_has_acl = 1;
}

char	acl_indicator(const t_file *f)
{
	if (f->acl_type == ACL_CONTEXT)
		return ('.');
	if (f->acl_type == ACL_YES)
		return ('+');
	if (f->acl_type == ACL_UNKNOWN || !f->stat_ok)
		return ('?');
	return (' ');
}
