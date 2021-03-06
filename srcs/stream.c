/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   stream.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ivankozlov <ivankozlov@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/05/02 10:32:16 by ivankozlov        #+#    #+#             */
/*   Updated: 2019/05/04 15:14:37 by ivankozlov       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ssl.h"
#include "structs.h"
#include "ftstream.h"

#include "memory.h"
#include "ftstring.h"

#include <fcntl.h>
#include <sys/stat.h>

int					g_printed_file = 0;
int					g_printed_stdin = 0;
int					g_printed_string = 0;

t_stream			stream_fd(char *filename)
{
	t_stream		s;
	struct stat		stat;

	s.fd = filename ? open(filename, O_RDONLY) : 0;
	s.string = NULL;
	s.filename = filename;
	s.content = s.fd == 0 ? string_init(0x4000) : NULL;
	if (s.fd < 0)
		error_handler(ERR_FILE_NOT_FOUND, 0, filename);
	if (s.fd >= 0 && (fstat(s.fd, &stat)) == -1)
		error_handler(-1, 1, NULL);
	if (S_ISDIR(stat.st_mode))
	{
		error_handler(ERR_GOT_DIR, 0, filename);
		s.fd = -1;
	}
	g_printed_file = g_printed_file || filename;
	g_printed_stdin = g_printed_file || !filename;
	return (s);
}

t_stream			stream_str(char *string)
{
	t_stream		s;

	s.fd = -1;
	s.pos = 0;
	s.content = NULL;
	s.filename = NULL;
	s.string = string;
	s.size = ft_strlen(string);
	g_printed_string = 1;
	return (s);
}

t_digest			hash_stream(t_stream stream,
	t_ssl_main main, t_print_digest *cb)
{
	t_digest	d;
	int			eos;
	int			last;
	size_t		total;
	t_chunk		*chunk;

	eos = 0;
	last = 0;
	total = 0;
	if (stream.fd < 0 && !stream.string)
		return (g_error_digest);
	d = main.init_digest();
	while (!last)
	{
		chunk = get_chunk_stream(stream, main.info.chunk_size);
		total += chunk->size;
		last = prepare_chunk(chunk, total, &eos, main.info.big_endian);
		main.hash(chunk->msg, d);
		ft_free(2, chunk->msg, chunk);
	}
	if (cb)
		(*cb)(main, d, stream);
	DOIFTRUE(stream.content, string_destroy(stream.content, FALSE));
	return (d);
}
