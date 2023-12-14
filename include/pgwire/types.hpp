#pragma once

#include <cstdint>
#include <vector>
namespace pgwire {

using Byte = uint8_t;
using Bytes = std::vector<Byte>;
using MessageTag = Byte;

using size_t = std::size_t;
using oid_t = int32_t;

enum class FrontendType {
    Invalid,
    Startup,
    SSLRequest,
    Bind,
    Close,
    CopyFail,
    Describe,
    Execute,
    Flush,
    FunctionCall,
    Parse,
    Query,
    Sync,
    Terminate,
    GSSResponse,
    SASLResponse,
    SASLInitialResponse,
};

enum class FormatCode : uint16_t {
    Text = 0,
    Binary = 1,
};

enum class Oid : int32_t {
    Bool = 16,
    Bytea = 17,
    Char = 18,
    Name = 19,
    Int8 = 20,
    Int2 = 21,
    Int2Vector = 22,
    Int4 = 23,
    Regproc = 24,
    Text = 25,
    Oid = 26,
    Tid = 27,
    Xid = 28,
    Cid = 29,
    OidVector = 30,
    PgDdlCommand = 32,
    PgType = 71,
    PgAttribute = 75,
    PgProc = 81,
    PgClass = 83,
    Json = 114,
    Xml = 142,
    XmlArray = 143,
    PgNodeTree = 194,
    JsonArray = 199,
    PgTypeArray = 210,
    TableAmHandler = 269,
    PgAttributeArray = 270,
    Xid8Array = 271,
    PgProcArray = 272,
    PgClassArray = 273,
    IndexAmHandler = 325,
    Point = 600,
    Lseg = 601,
    Path = 602,
    Box = 603,
    Polygon = 604,
    Line = 628,
    LineArray = 629,
    Cidr = 650,
    CidrArray = 651,
    Float4 = 700,
    Float8 = 701,
    Unknown = 705,
    Circle = 718,
    CircleArray = 719,
    MacAddr8 = 774,
    MacAddr8Array = 775,
    Money = 790,
    MoneyArray = 791,
    MacAddr = 829,
    Inet = 869,
    BoolArray = 1000,
    ByteaArray = 1001,
    CharArray = 1002,
    NameArray = 1003,
    Int2Array = 1005,
    Int2VectorArray = 1006,
    Int4Array = 1007,
    RegprocArray = 1008,
    TextArray = 1009,
    TidArray = 1010,
    XidArray = 1011,
    CidArray = 1012,
    OidVectorArray = 1013,
    BpcharArray = 1014,
    VarcharArray = 1015,
    Int8Array = 1016,
    PointArray = 1017,
    LsegArray = 1018,
    PathArray = 1019,
    BoxArray = 1020,
    Float4Array = 1021,
    Float8Array = 1022,
    PolygonArray = 1027,
    OidArray = 1028,
    Aclitem = 1033,
    AclitemArray = 1034,
    MacAddrArray = 1040,
    InetArray = 1041,
    Bpchar = 1042,
    Varchar = 1043,
    Date = 1082,
    Time = 1083,
    Timestamp = 1114,
    TimestampArray = 1115,
    DateArray = 1182,
    TimeArray = 1183,
    TimestampTz = 1184,
    TimestampTzArray = 1185,
    Interval = 1186,
    IntervalArray = 1187,
    NumericArray = 1231,
    PgDatabase = 1248,
    CstringArray = 1263,
    TimeTz = 1266,
    TimeTzArray = 1270,
    Bit = 1560,
    BitArray = 1561,
    Varbit = 1562,
    VarbitArray = 1563,
    Numeric = 1700,
    RefCursor = 1790,
    RefCursorArray = 2201,
    RegProcedure = 2202,
    RegOper = 2203,
    RegOperator = 2204,
    Regclass = 2205,
    Regtype = 2206,
    RegProcedureArray = 2207,
    RegOperArray = 2208,
    RegOperatorArray = 2209,
    RegclassArray = 2210,
    RegtypeArray = 2211,
    Record = 2249,
    Cstring = 2275,
    Any = 2276,
    AnyArray = 2277,
    Void = 2278,
    Trigger = 2279,
    LanguageHandler = 2280,
    Internal = 2281,
    Anyelement = 2283,
    RecordArray = 2287,
    AnynonArray = 2776,
    PgAuthid = 2842,
    PgAuthMembers = 2843,
    TxidSnapshotArray = 2949,
    Uuid = 2950,
    UuidArray = 2951,
    TxidSnapshot = 2970,
    FdwHandler = 3115,
    PgLsn = 3220,
    PgLsnArray = 3221,
    TsmHandler = 3310,
    PgNdistinct = 3361,
    PgDependencies = 3402,
    Anyenum = 3500,
    TsVector = 3614,
    Tsquery = 3615,
    GtsVector = 3642,
    TsVectorArray = 3643,
    GtsVectorArray = 3644,
    TsqueryArray = 3645,
    Regconfig = 3734,
    RegconfigArray = 3735,
    Regdictionary = 3769,
    RegdictionaryArray = 3770,
    Jsonb = 3802,
    JsonbArray = 3807,
};

int16_t get_oid_size(Oid oid);

} // namespace pgwire