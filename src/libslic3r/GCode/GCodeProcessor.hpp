#ifndef slic3r_GCodeProcessor_hpp_
#define slic3r_GCodeProcessor_hpp_

#if ENABLE_GCODE_VIEWER
#include "../GCodeReader.hpp"
#include "../Point.hpp"
#include "../ExtrusionEntity.hpp"

#include <array>

namespace Slic3r {

    class GCodeProcessor
    {
    public:
        static const std::string Extrusion_Role_Tag;
        static const std::string Width_Tag;
        static const std::string Height_Tag;
        static const std::string Mm3_Per_Mm_Tag;

    private:
        using AxisCoords = std::array<float, 4>;

        enum class EUnits : unsigned char
        {
            Millimeters,
            Inches
        };

        enum class EPositioningType : unsigned char
        {
            Absolute,
            Relative
        };

        enum class EMoveType : unsigned char
        {
            Noop,
            Retract,
            Unretract,
            Tool_change,
            Travel,
            Extrude,
            Num_Types
        };

        struct CachedPosition
        {
            AxisCoords position; // mm
            float feedrate;  // mm/s
        };

    public:
        struct MoveVertex
        {
            EMoveType type{ EMoveType::Noop };
            ExtrusionRole extrusion_role{ erNone };
            Vec3f position{ Vec3f::Zero() }; // mm
            float feedrate{ 0.0f }; // mm/s
            float width{ 0.0f }; // mm
            float height{ 0.0f }; // mm
            float mm3_per_mm{ 0.0f };
            float fan_speed{ 0.0f }; // percentage
            unsigned int extruder_id{ 0 };

            std::string to_string() const
            {
                std::string str = std::to_string((int)type);
                str += ", " + std::to_string((int)extrusion_role);
                str += ", " + Slic3r::to_string((Vec3d)position.cast<double>());
                str += ", " + std::to_string(extruder_id);
                str += ", " + std::to_string(feedrate);
                str += ", " + std::to_string(width);
                str += ", " + std::to_string(height);
                str += ", " + std::to_string(mm3_per_mm);
                str += ", " + std::to_string(fan_speed);
                return str;
            }
        };

        struct Result
        {
            std::vector<MoveVertex> moves;
            void reset() { moves = std::vector<MoveVertex>(); }
        };

    private:
        GCodeReader m_parser;

        EUnits m_units;
        EPositioningType m_global_positioning_type;
        EPositioningType m_e_local_positioning_type;
        std::vector<Vec3f> m_extruder_offsets;
        GCodeFlavor m_flavor;

        AxisCoords m_start_position; // mm
        AxisCoords m_end_position;   // mm
        AxisCoords m_origin;         // mm
        CachedPosition m_cached_position;

        float m_feedrate;  // mm/s
        float m_width;     // mm
        float m_height;    // mm
        float m_mm3_per_mm;
        float m_fan_speed; // percentage
        ExtrusionRole m_extrusion_role;
        unsigned int m_extruder_id;

        Result m_result;

    public:
        GCodeProcessor() { reset(); }

        void apply_config(const PrintConfig& config);
        void reset();

        const Result& get_result() const { return m_result; }
        Result&& extract_result() { return std::move(m_result); }

        // Process the gcode contained in the file with the given filename
        void process_file(const std::string& filename);

    private:
        void process_gcode_line(const GCodeReader::GCodeLine& line);

        // Process tags embedded into comments
        void process_tags(const std::string& comment);

        // Move
        void process_G1(const GCodeReader::GCodeLine& line);

        // Retract
        void process_G10(const GCodeReader::GCodeLine& line);

        // Unretract
        void process_G11(const GCodeReader::GCodeLine& line);

        // Firmware controlled Retract
        void process_G22(const GCodeReader::GCodeLine& line);

        // Firmware controlled Unretract
        void process_G23(const GCodeReader::GCodeLine& line);

        // Set to Absolute Positioning
        void process_G90(const GCodeReader::GCodeLine& line);

        // Set to Relative Positioning
        void process_G91(const GCodeReader::GCodeLine& line);

        // Set Position
        void process_G92(const GCodeReader::GCodeLine& line);

        // Set extruder to absolute mode
        void process_M82(const GCodeReader::GCodeLine& line);

        // Set extruder to relative mode
        void process_M83(const GCodeReader::GCodeLine& line);

        // Set fan speed
        void process_M106(const GCodeReader::GCodeLine& line);

        // Disable fan
        void process_M107(const GCodeReader::GCodeLine& line);

        // Set tool (Sailfish)
        void process_M108(const GCodeReader::GCodeLine& line);

        // Recall stored home offsets
        void process_M132(const GCodeReader::GCodeLine& line);

        // Set tool (MakerWare)
        void process_M135(const GCodeReader::GCodeLine& line);

        // Repetier: Store x, y and z position
        void process_M401(const GCodeReader::GCodeLine& line);

        // Repetier: Go to stored position
        void process_M402(const GCodeReader::GCodeLine& line);

        // Processes T line (Select Tool)
        void process_T(const GCodeReader::GCodeLine& line);
        void process_T(const std::string& command);

        void store_move_vertex(EMoveType type);
   };

} /* namespace Slic3r */

#endif // ENABLE_GCODE_VIEWER

#endif /* slic3r_GCodeProcessor_hpp_ */


