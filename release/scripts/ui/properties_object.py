# ##### BEGIN GPL LICENSE BLOCK #####
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software Foundation,
#  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# ##### END GPL LICENSE BLOCK #####

# <pep8 compliant>
import bpy
from rna_prop_ui import PropertyPanel


class ObjectButtonsPanel():
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "object"


class OBJECT_PT_context_object(ObjectButtonsPanel, bpy.types.Panel):
    bl_label = ""
    bl_show_header = False

    def draw(self, context):
        layout = self.layout
        space = context.space_data
        ob = context.object

        row = layout.row()
        row.label(text="", icon='OBJECT_DATA')
        if space.use_pin_id:
            row.template_ID(space, "pin_id")
        else:
            row.prop(ob, "name", text="")


class OBJECT_PT_transform(ObjectButtonsPanel, bpy.types.Panel):
    bl_label = "Transform"

    def draw(self, context):
        layout = self.layout

        ob = context.object

        row = layout.row()

        row.column().prop(ob, "location")
        if ob.rotation_mode == 'QUATERNION':
            row.column().prop(ob, "rotation_quaternion", text="Rotation")
        elif ob.rotation_mode == 'AXIS_ANGLE':
            #row.column().label(text="Rotation")
            #row.column().prop(pchan, "rotation_angle", text="Angle")
            #row.column().prop(pchan, "rotation_axis", text="Axis")
            row.column().prop(ob, "rotation_axis_angle", text="Rotation")
        else:
            row.column().prop(ob, "rotation_euler", text="Rotation")

        row.column().prop(ob, "scale")

        layout.prop(ob, "rotation_mode")


class OBJECT_PT_transform_locks(ObjectButtonsPanel, bpy.types.Panel):
    bl_label = "Transform Locks"
    bl_default_closed = True

    def draw(self, context):
        layout = self.layout

        ob = context.object

        row = layout.row()

        col = row.column()
        col.prop(ob, "lock_location", text="Location")

        col = row.column()
        if ob.rotation_mode in ('QUATERNION', 'AXIS_ANGLE'):
            col.prop(ob, "lock_rotations_4d", text="Rotation")
            if ob.lock_rotations_4d:
                col.prop(ob, "lock_rotation_w", text="W")
            col.prop(ob, "lock_rotation", text="")
        else:
            col.prop(ob, "lock_rotation", text="Rotation")

        row.column().prop(ob, "lock_scale", text="Scale")


class OBJECT_PT_relations(ObjectButtonsPanel, bpy.types.Panel):
    bl_label = "Relations"

    def draw(self, context):
        layout = self.layout

        ob = context.object

        split = layout.split()

        col = split.column()
        col.prop(ob, "layers")
        col.separator()
        col.prop(ob, "pass_index")

        col = split.column()
        col.label(text="Parent:")
        col.prop(ob, "parent", text="")

        sub = col.column()
        sub.prop(ob, "parent_type", text="")
        parent = ob.parent
        if parent and ob.parent_type == 'BONE' and parent.type == 'ARMATURE':
            sub.prop_object(ob, "parent_bone", parent.data, "bones", text="")
        sub.active = (parent is not None)


class OBJECT_PT_groups(ObjectButtonsPanel, bpy.types.Panel):
    bl_label = "Groups"

    def draw(self, context):
        layout = self.layout

        ob = context.object

        row = layout.row(align=True)
        row.operator("object.group_link", text="Add to Group")
        row.operator("object.group_add", text="", icon='ZOOMIN')

        # XXX, this is bad practice, yes, I wrote it :( - campbell
        index = 0
        value = str(tuple(context.scene.cursor_location))
        for group in bpy.data.groups:
            if ob.name in group.objects:
                col = layout.column(align=True)

                col.set_context_pointer("group", group)

                row = col.box().row()
                row.prop(group, "name", text="")
                row.operator("object.group_remove", text="", icon='X', emboss=False)

                split = col.box().split()

                col = split.column()
                col.prop(group, "layer", text="Dupli")

                col = split.column()
                col.prop(group, "dupli_offset", text="")

                prop = col.operator("wm.context_set_value", text="From Cursor")
                prop.data_path = "object.users_group[%d].dupli_offset" % index
                prop.value = value
                index += 1


class OBJECT_PT_display(ObjectButtonsPanel, bpy.types.Panel):
    bl_label = "Display"

    def draw(self, context):
        layout = self.layout

        ob = context.object

        split = layout.split()
        col = split.column()
        col.prop(ob, "max_draw_type", text="Type")

        col = split.column()
        row = col.row()
        row.prop(ob, "draw_bounds", text="Bounds")
        sub = row.row()
        sub.active = ob.draw_bounds
        sub.prop(ob, "draw_bounds_type", text="")

        split = layout.split()

        col = split.column()
        col.prop(ob, "draw_name", text="Name")
        col.prop(ob, "draw_axis", text="Axis")
        col.prop(ob, "draw_wire", text="Wire")
        col.prop(ob, "color", text="Object Color")

        col = split.column()
        col.prop(ob, "draw_texture_space", text="Texture Space")
        col.prop(ob, "x_ray", text="X-Ray")
        col.prop(ob, "draw_transparent", text="Transparency")


class OBJECT_PT_duplication(ObjectButtonsPanel, bpy.types.Panel):
    bl_label = "Duplication"

    def draw(self, context):
        layout = self.layout

        ob = context.object

        layout.prop(ob, "dupli_type", expand=True)

        if ob.dupli_type == 'FRAMES':
            split = layout.split()

            col = split.column(align=True)
            col.prop(ob, "dupli_frames_start", text="Start")
            col.prop(ob, "dupli_frames_end", text="End")

            col = split.column(align=True)
            col.prop(ob, "dupli_frames_on", text="On")
            col.prop(ob, "dupli_frames_off", text="Off")

            layout.prop(ob, "use_dupli_frames_speed", text="Speed")

        elif ob.dupli_type == 'VERTS':
            layout.prop(ob, "use_dupli_verts_rotation", text="Rotation")

        elif ob.dupli_type == 'FACES':
            split = layout.split()

            col = split.column()
            col.prop(ob, "use_dupli_faces_scale", text="Scale")

            col = split.column()
            col.prop(ob, "dupli_faces_scale", text="Inherit Scale")

        elif ob.dupli_type == 'GROUP':
            layout.prop(ob, "dupli_group", text="Group")


# XXX: the following options are all quite buggy, ancient hacks that should be dropped

class OBJECT_PT_animation(ObjectButtonsPanel, bpy.types.Panel):
    bl_label = "Animation Hacks"
    bl_default_closed = True

    def draw(self, context):
        layout = self.layout

        ob = context.object

        split = layout.split()

        col = split.column()
        col.label(text="Time Offset:")
        col.prop(ob, "time_offset_edit", text="Edit")
        row = col.row()
        row.prop(ob, "time_offset_particle", text="Particle")
        row.active = len(ob.particle_systems) != 0
        row = col.row()
        row.prop(ob, "time_offset_parent", text="Parent")
        row.active = (ob.parent is not None)
        row = col.row()
        row.prop(ob, "slow_parent")
        row.active = (ob.parent is not None)
        col.prop(ob, "time_offset", text="Offset")

        # XXX: these are still used for a few curve-related tracking features
        col = split.column()
        col.label(text="Tracking Axes:")
        col.prop(ob, "track_axis", text="Axis")
        col.prop(ob, "up_axis", text="Up Axis")

from properties_animviz import MotionPathButtonsPanel, OnionSkinButtonsPanel


class OBJECT_PT_motion_paths(MotionPathButtonsPanel, bpy.types.Panel):
    #bl_label = "Object Motion Paths"
    bl_context = "object"

    @classmethod
    def poll(cls, context):
        return (context.object)

    def draw(self, context):
        layout = self.layout

        ob = context.object

        self.draw_settings(context, ob.animation_visualisation)

        layout.separator()

        split = layout.split()

        col = split.column()
        col.operator("object.paths_calculate", text="Calculate Paths")

        col = split.column()
        col.operator("object.paths_clear", text="Clear Paths")


class OBJECT_PT_onion_skinning(OnionSkinButtonsPanel): #, bpy.types.Panel): # inherit from panel when ready
    #bl_label = "Object Onion Skinning"
    bl_context = "object"

    @classmethod
    def poll(cls, context):
        return (context.object)

    def draw(self, context):
        layout = self.layout

        ob = context.object

        self.draw_settings(context, ob.animation_visualisation)


class OBJECT_PT_custom_props(ObjectButtonsPanel, PropertyPanel, bpy.types.Panel):
    COMPAT_ENGINES = {'BLENDER_RENDER', 'BLENDER_GAME'}
    _context_path = "object"


def register():
    pass


def unregister():
    pass

if __name__ == "__main__":
    register()
