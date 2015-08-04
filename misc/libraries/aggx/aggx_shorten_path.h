//----------------------------------------------------------------------------
// Anti-Grain Geometry (AGG) - Version 2.5
// A high quality rendering engine for C++
// Copyright (C) 2002-2006 Maxim Shemanarev
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://antigrain.com
// 
// AGG is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// AGG is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AGG; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
// MA 02110-1301, USA.
//----------------------------------------------------------------------------

#ifndef AGGX_SHORTEN_PATH_INCLUDED
#define AGGX_SHORTEN_PATH_INCLUDED

#include "basics.h"
#include "aggx_vertex_sequence.h"

namespace aggx
{

	//===========================================================shorten_path
	template<class VertexSequence> 
	void shorten_path(VertexSequence& vs, real s, unsigned closed = 0)
	{
		typedef typename VertexSequence::value_type vertex_type;

		if(s > 0.0f && vs.size() > 1)
		{
			real d;
			int n = int(vs.size() - 2);
			while(n)
			{
				d = vs[n].dist;
				if(d > s) break;
				vs.pop_back();
				s -= d;
				--n;
			}
			if(vs.size() < 2)
			{
				vs.clear();
			}
			else
			{
				n = vs.size() - 1;
				vertex_type& prev = vs[n-1];
				vertex_type& last = vs[n];
				d = (prev.dist - s) / prev.dist;
				real x = prev.x + (last.x - prev.x) * d;
				real y = prev.y + (last.y - prev.y) * d;
				last.x = x;
				last.y = y;
				if(!prev(last)) vs.pop_back();
				vs.close(closed != 0);
			}
		}
	}
}

#endif
