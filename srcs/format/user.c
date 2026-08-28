#include "ft_ls.h"

// cache user and group to avoid repeated call to getpwuid/getgrgid
// return NULL when there is no entry: the caller needs to know, because ls
// pads a name on the right but a raw numeric id on the left
const char	*user_name(uid_t uid)
{
	static uid_t	cached_uid;
	static char		*cached_name;
	struct passwd	*pw;

	if (cached_name && cached_uid == uid)
		return (cached_name);
	pw = getpwuid(uid);
	if (pw && pw->pw_name)
	{
		cached_uid = uid;
		cached_name = pw->pw_name;
		return (cached_name);
	}
	return (NULL);
}

const char	*group_name(gid_t gid)
{
	static gid_t	cached_gid;
	static char		*cached_name;
	struct group	*gr;

	if (cached_name && cached_gid == gid)
		return (cached_name);
	gr = getgrgid(gid);
	if (gr && gr->gr_name)
	{
		cached_gid = gid;
		cached_name = gr->gr_name;
		return (cached_name);
	}
	return (NULL);
}

const char	*owner_field(const t_file *f)
{
	static char	buf[32];
	const char	*name;

	if (!f->stat_ok)
		return ("?");
	name = NULL;
	if (!g_numeric_ids)
		name = user_name(f->st.st_uid);
	if (name)
		return (name);
	snprintf(buf, sizeof(buf), "%u", (unsigned int)f->st.st_uid);
	return (buf);
}

const char	*group_field(const t_file *f)
{
	static char	buf[32];
	const char	*name;

	if (!f->stat_ok)
		return ("?");
	name = NULL;
	if (!g_numeric_ids)
		name = group_name(f->st.st_gid);
	if (name)
		return (name);
	snprintf(buf, sizeof(buf), "%u", (unsigned int)f->st.st_gid);
	return (buf);
}

// ls right-aligns a field that holds a raw id instead of a name
int	owner_is_id(const t_file *f)
{
	if (!f->stat_ok)
		return (0);
	return (g_numeric_ids || !user_name(f->st.st_uid));
}

int	group_is_id(const t_file *f)
{
	if (!f->stat_ok)
		return (0);
	return (g_numeric_ids || !group_name(f->st.st_gid));
}
