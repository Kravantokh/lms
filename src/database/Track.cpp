/*
 * Copyright (C) 2013 Emeric Poupon
 *
 * This file is part of LMS.
 *
 * LMS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LMS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LMS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <boost/foreach.hpp>

#include "SqlQuery.hpp"

#include "AudioTypes.hpp"

namespace Database {

Track::Track(const boost::filesystem::path& p, Artist::pointer artist, Release::pointer release)
:
_trackNumber(0),
_discNumber(0),
_filePath( p.string() ),
_artist(artist),
_release(release)
{
}


void
Track::setGenres(std::vector<Genre::pointer> genres)
{
	if (_genres.size())
		_genres.clear();

	BOOST_FOREACH(Genre::pointer genre, genres) {
		_genres.insert( genre );
	}
}

Track::pointer
Track::getByPath(Wt::Dbo::Session& session, const boost::filesystem::path& p)
{
	return session.find<Track>().where("path = ?").bind(p.string());
}

Track::pointer
Track::getById(Wt::Dbo::Session& session, id_type id)
{
	return session.find<Track>().where("id = ?").bind(id);
}


Track::pointer
Track::create(Wt::Dbo::Session& session, const boost::filesystem::path& p, Artist::pointer artist, Release::pointer release)
{
	return session.add(new Track(p, artist, release) );
}

Wt::Dbo::collection< Track::pointer >
Track::getAll(Wt::Dbo::Session& session)
{
	return session.find<Track>();
}

std::vector< Genre::pointer >
Track::getGenres(void) const
{
	std::vector< Genre::pointer > genres;
	std::copy(_genres.begin(), _genres.end(), std::back_inserter(genres));
	return genres;
}

Wt::Dbo::collection< Track::pointer >
Track::getAll(Wt::Dbo::Session& session,
			const std::vector<Artist::id_type>& artistIds,
			const std::vector<Release::id_type>& releaseIds,
			const std::vector<Genre::id_type>& genreIds,
			int offset, int size)
{
	std::string sqlQuery = "SELECT t FROM track t";

	if (!artistIds.empty())
		sqlQuery += " INNER JOIN artist a ON a.id = t.artist_id";

	if (!releaseIds.empty())
		sqlQuery += " INNER JOIN release r ON r.id = t.release_id";

	if (!genreIds.empty())
	{
		sqlQuery += " INNER JOIN genre g ON g.id = t_g.genre_id";
		sqlQuery += " INNER JOIN track_genre t_g ON t_g.track_id = t.id AND t_g.genre_id = g.id";
	}

	WhereClause where;

	{
		WhereClause artistWhere;

		for (std::size_t i = 0; i < artistIds.size(); ++i)
			artistWhere.Or( WhereClause("a.id = ?") );

		where.And(artistWhere);
	}

	{
		WhereClause releaseWhere;

		for (std::size_t i = 0; i < releaseIds.size(); ++i)
			releaseWhere.Or( WhereClause("r.id = ?") );

		where.And(releaseWhere);
	}

	{
		WhereClause genreWhere;

		for (std::size_t i = 0; i < genreIds.size(); ++i)
			genreWhere.Or( WhereClause("g.id = ?") );

		where.And(genreWhere);
	}


	Wt::Dbo::Query<Track::pointer> query = session.query<Track::pointer>( sqlQuery + " " + where.get() ).offset(offset).limit(size);

	BOOST_FOREACH(const Artist::id_type artistId, artistIds)
		query.bind(artistId);

	BOOST_FOREACH(const Release::id_type releaseId, releaseIds)
		query.bind(releaseId);

	BOOST_FOREACH(const Genre::id_type genreId, genreIds)
		query.bind(genreId);

	query.groupBy("t");

	return query;
}

} // namespace Database



