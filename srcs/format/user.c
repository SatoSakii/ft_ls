#include "ft_ls.h"

// cache user and group to avoid repeated call to getpwuid/getgrgid
const char	*user_name(uid_t uid)
{
	static char		buf[32];
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
	snprintf(buf, sizeof(buf), "%u", (unsigned int)uid);
	return (buf);
}

const char	*group_name(gid_t gid)
{
	static char		buf[32];
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
	snprintf(buf, sizeof(buf), "%u", (unsigned int)gid);
	return (buf);
}

const char	*owner_field(const t_file *f)
{
	static char	buf[32];

	if (g_numeric_ids)
	{
		snprintf(buf, sizeof(buf), "%u", (unsigned int)f->st.st_uid);
		return (buf);
	}
	return (user_name(f->st.st_uid));
}

const char	*group_field(const t_file *f)
{
	static char	buf[32];

	if (g_numeric_ids)
	{
		snprintf(buf, sizeof(buf), "%u", (unsigned int)f->st.st_gid);
		return (buf);
	}
	return (group_name(f->st.st_gid));
}
