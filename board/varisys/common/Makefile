#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

ifneq ($(OBJTREE),$(SRCTREE))
$(shell mkdir -p $(obj)board/varisys/common)
endif

LIB	= $(obj)libvarisys.o

COBJS-$(CONFIG_FMAN_ENET)	+= fman.o
COBJS-$(CONFIG_ID_EEPROM)	+= sys_eeprom.o

# deal with common files for P-series corenet based devices
SUBLIB-$(CONFIG_CYRUS)		+= p_corenet/libp_corenet.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS-y))
SOBJS	:= $(addprefix $(obj),$(SOBJS))
SUBLIB	:= $(addprefix $(obj),$(SUBLIB-y))

$(LIB):	$(obj).depend $(OBJS) $(SUBLIB)
	$(call cmd_link_o_target, $(OBJS) $(SUBLIB))

$(SUBLIB): $(obj).depend
	$(MAKE) -C $(dir $(subst $(obj),,$@))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
