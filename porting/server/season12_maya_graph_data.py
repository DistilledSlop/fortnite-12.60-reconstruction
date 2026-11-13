"""Season 12 Maya reward graph and cosmetic variant metadata.

Derived from the ignored maya-graph-reports and maya-variant-token-metadata
normalized reports. Boolean fields are serialized asset values: None means
omitted, not false. The recovered token constructor defaults (auto equip=True,
mark unseen=True, giftbox=False) are separate from serialized presence.
Backend validation and reconciliation rules are local policies. Native color
format and the cooked default were recovered separately; see RICH_COLOR below.
"""
from dataclasses import dataclass
from typing import Optional

GRAPH_TEMPLATE = "AthenaRewardGraph:S12_HeroSelections"
GRAPH_TYPE = "AthenaRewardEventGraph"
PURCHASE_TOKEN = "RewardEventGraphPurchaseToken:S12_HeroSelections_PurchaseToken"
TARGET_ITEM = "CID_695_Athena_Commando_F_DesertOpsCamo"
GRAPH_SOURCE_SHA256 = {'uasset': '23a5b990fa00145c8283de373d30cd2e6db8dac1470ac18675ebd7a824b2cda5', 'uexp': '7690c6afcd49faebf1afbea2f58c686d1d004a20d0b66ebd213db3f3dbe873bb'}
TARGET_SOURCE_SHA256 = {'.uasset': 'b1ef05b39cf6dbf2c31c12a2a035b7e897b64cb8d9d6762b9d99ba77b5c13ba8', '.uexp': '5255ba98fc666c4c09bc02fd0c74080994afc905725b38d0cfda1960af71f5e8'}
TOKEN_CONSTRUCTOR_DEFAULTS = {"auto_equip": True, "mark_unseen": True, "create_giftbox": False}

@dataclass(frozen=True)
class TierKey:
    token: str
    tag: str
    maximum: int
    initial: int

@dataclass(frozen=True)
class Channel:
    export: str
    label: str
    # (property tag, is_default), including options not granted by this graph.
    options: tuple[tuple[str, bool], ...]

@dataclass(frozen=True)
class Node:
    node_id: str
    tier_key: str
    reward_token: str
    quantity: int
    channel: str
    unlocked_tag: str
    is_default: bool
    auto_equip: Optional[bool]
    mark_unseen: Optional[bool]
    create_giftbox: Optional[bool]
    never_persisted: Optional[bool]
    source_uasset_sha256: str
    source_uexp_sha256: str
    key_count: int
    minimum_key_count: int
    days: int
    required_ownership: bool
    operation: str
    context: str

TIER_KEYS = {key.tag: key for key in (
    TierKey('AthenaToken_BRS12_HeroSelection_01', 'ERG.Node.Tier01', 1, 0),
    TierKey('AthenaToken_BRS12_HeroSelection_02', 'ERG.Node.Tier02', 1, 0),
    TierKey('AthenaToken_BRS12_HeroSelection_03', 'ERG.Node.Tier03', 1, 0),
    TierKey('AthenaToken_BRS12_HeroSelection_04', 'ERG.Node.Tier04', 1, 0),
    TierKey('AthenaToken_BRS12_HeroSelection_05', 'ERG.Node.Tier05', 1, 0),
    TierKey('AthenaToken_BRS12_HeroSelection_06', 'ERG.Node.Tier06', 1, 0),
    TierKey('AthenaToken_BRS12_HeroSelection_07', 'ERG.Node.Tier07', 1, 0),
    TierKey('AthenaToken_BRS12_HeroSelection_08', 'ERG.Node.Tier08', 1, 0),
    TierKey('AthenaToken_BRS12_HeroSelection_09', 'ERG.Node.Tier09', 1, 0),
    TierKey('AthenaToken_BRS12_HeroSelection_10', 'ERG.Node.Tier10', 1, 0),
)}

CHANNELS = {
    'Cosmetics.Variant.Channel.Particle': Channel('FortCosmeticCharacterPartVariant_0', 'BOOTS', (
        ('Cosmetics.Variant.Property.Particle1', True),
        ('Cosmetics.Variant.Property.Particle2', False),
        ('Cosmetics.Variant.Property.Particle3', False),
        ('Cosmetics.Variant.Property.Particle4', False),
    )),
    'Cosmetics.Variant.Channel.Progressive': Channel('FortCosmeticCharacterPartVariant_1', 'VEST', (
        ('Cosmetics.Variant.Property.Stage1', True),
        ('Cosmetics.Variant.Property.Stage2', False),
        ('Cosmetics.Variant.Property.Stage3', False),
        ('Cosmetics.Variant.Property.Stage4', False),
        ('Cosmetics.Variant.Property.Stage5', False),
    )),
    'Cosmetics.Variant.Channel.ClothingColor': Channel('FortCosmeticCharacterPartVariant_2', 'SLEEVES', (
        ('Cosmetics.Variant.Property.Color.003', True),
        ('Cosmetics.Variant.Property.Color.004', False),
        ('Cosmetics.Variant.Property.Color.005', False),
    )),
    'Cosmetics.Variant.Channel.Parts': Channel('FortCosmeticCharacterPartVariant_3', 'HELMET', (
        ('Cosmetics.Variant.Property.Stage6', True),
        ('Cosmetics.Variant.Property.Stage7', False),
        ('Cosmetics.Variant.Property.Stage8', False),
        ('Cosmetics.Variant.Property.Stage9', False),
        ('Cosmetics.Variant.Property.Stage10', False),
        ('Cosmetics.Variant.Property.Stage11', False),
    )),
    'Cosmetics.Variant.Channel.Hair': Channel('FortCosmeticCharacterPartVariant_4', 'HAIRSTYLE', (
        ('Cosmetics.Variant.Property.Color.014', True),
        ('Cosmetics.Variant.Property.Color.015', False),
        ('Cosmetics.Variant.Property.Color.016', False),
        ('Cosmetics.Variant.Property.Color.017', False),
    )),
    'Cosmetics.Variant.Channel.Material': Channel('FortCosmeticMaterialVariant_0', 'SHIRT COLORS', (
        ('Cosmetics.Variant.Property.Mat1', True),
        ('Cosmetics.Variant.Property.Mat2', False),
        ('Cosmetics.Variant.Property.Mat3', False),
        ('Cosmetics.Variant.Property.Mat4', False),
        ('Cosmetics.Variant.Property.Mat5', False),
        ('Cosmetics.Variant.Property.Mat6', False),
        ('Cosmetics.Variant.Property.Mat7', False),
        ('Cosmetics.Variant.Property.Mat8', False),
        ('Cosmetics.Variant.Property.Mat17', False),
        ('Cosmetics.Variant.Property.Mat18', False),
        ('Cosmetics.Variant.Property.Mat19', False),
        ('Cosmetics.Variant.Property.Mat20', False),
        ('Cosmetics.Variant.Property.Mat21', False),
        ('Cosmetics.Variant.Property.Mat22', False),
        ('Cosmetics.Variant.Property.Mat23', False),
        ('Cosmetics.Variant.Property.Mat24', False),
    )),
    'Cosmetics.Variant.Channel.JerseyColor': Channel('FortCosmeticMaterialVariant_1', 'PANTS COLOR', (
        ('Cosmetics.Variant.Property.Mat9', True),
        ('Cosmetics.Variant.Property.Mat10', False),
        ('Cosmetics.Variant.Property.Mat11', False),
        ('Cosmetics.Variant.Property.Mat12', False),
        ('Cosmetics.Variant.Property.Mat13', False),
        ('Cosmetics.Variant.Property.Mat14', False),
        ('Cosmetics.Variant.Property.Mat15', False),
        ('Cosmetics.Variant.Property.Mat16', False),
    )),
    'Cosmetics.Variant.Channel.Emissive': Channel('FortCosmeticMaterialVariant_2', 'TATTOOS', (
        ('Cosmetics.Variant.Property.Emissive1', True),
        ('Cosmetics.Variant.Property.Emissive2', False),
        ('Cosmetics.Variant.Property.Emissive3', False),
        ('Cosmetics.Variant.Property.Emissive4', False),
        ('Cosmetics.Variant.Property.Emissive5', False),
    )),
    'Cosmetics.Variant.Channel.Pattern': Channel('FortCosmeticMaterialVariant_3', 'SCARF', (
        ('Cosmetics.Variant.Property.Color.001', True),
        ('Cosmetics.Variant.Property.Color.002', False),
    )),
    'Cosmetics.Variant.Channel.Mesh': Channel('FortCosmeticMaterialVariant_4', 'FACE PAINT', (
        ('Cosmetics.Variant.Property.Color.009', True),
        ('Cosmetics.Variant.Property.Color.010', False),
        ('Cosmetics.Variant.Property.Color.011', False),
        ('Cosmetics.Variant.Property.Color.012', False),
        ('Cosmetics.Variant.Property.Color.013', False),
    )),
}

# Rows follow graph order. Each token carries its own cooked-source hashes.
_NODES = (
    Node(
        'ERG.Node.Tier01.0', 'AthenaToken_BRS12_HeroSelection_01', 'VTID_529_DesertOpsCamo_Hair_StyleA', 1, 'Cosmetics.Variant.Channel.Hair', 'Cosmetics.Variant.Property.Color.014', True,
        False, None, None, True,
        'e221f28a84b0e2768b161c46ac81306a3eb3213846d5e25492a8cb3a53bb2470', 'ce402b3d20424ccd275bfec5e5f61123116d46581c643203af4efc70ae83ae84',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier01.1', 'AthenaToken_BRS12_HeroSelection_01', 'VTID_448_DesertOpsCamo_Hair_StyleB', 1, 'Cosmetics.Variant.Channel.Hair', 'Cosmetics.Variant.Property.Color.015', False,
        None, None, None, None,
        '99d2395da45881761d7c56de9166a8d73a6e3bb1c0ac41853f575c8f02cfc34f', 'cd86f86aedb1c542af0c88ac7438bc0470a980dbd3734d2f852cf6a3fa8d2b0b',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier01.2', 'AthenaToken_BRS12_HeroSelection_01', 'VTID_449_DesertOpsCamo_Hair_StyleC', 1, 'Cosmetics.Variant.Channel.Hair', 'Cosmetics.Variant.Property.Color.016', False,
        None, None, None, None,
        'ecd33dc9f3cfce9db7987fda14be6da5953f921e8a725aaafcdd5b988ab6d2d7', 'db9789efd6522465a328a285646839f411360faac5bc60b34f3bfdf3693df792',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier01.3', 'AthenaToken_BRS12_HeroSelection_01', 'VTID_450_DesertOpsCamo_Hair_StyleD', 1, 'Cosmetics.Variant.Channel.Hair', 'Cosmetics.Variant.Property.Color.017', False,
        None, None, None, None,
        'a5f3d8b80bdbf90c50c1df939ced6ac90973bd77c7949cc550d7fffc551074aa', 'b50e056016f4de9e343f0e22319efb8d95081feb1567c30ba2dd4c708e874a6a',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier02.0', 'AthenaToken_BRS12_HeroSelection_02', 'VTID_530_DesertOpsCamo_Tattoo_StyleA', 1, 'Cosmetics.Variant.Channel.Emissive', 'Cosmetics.Variant.Property.Emissive1', True,
        False, None, None, True,
        '1472811c9b3ccc31375075cb2ecc1694ced6b084f13c2f23dfcc07c63ba5a6f5', 'b0f3b4709454b21c2c44f62214a9c0c84ea5458ef95711dcbf6c286c2f830ab2',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier02.1', 'AthenaToken_BRS12_HeroSelection_02', 'VTID_451_DesertOpsCamo_Tattoo_StyleB', 1, 'Cosmetics.Variant.Channel.Emissive', 'Cosmetics.Variant.Property.Emissive2', False,
        None, None, None, None,
        '29651e74ea86acaf49ecb325656eeed1dfd172ccc063a4d79f710253aadb763e', '291d0357fee9362c241dea768a3da465f28e34a14991b7a99885bcd58a4b25ae',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier02.2', 'AthenaToken_BRS12_HeroSelection_02', 'VTID_452_DesertOpsCamo_Tattoo_StyleC', 1, 'Cosmetics.Variant.Channel.Emissive', 'Cosmetics.Variant.Property.Emissive3', False,
        None, None, None, None,
        '62d21b38d12eaf022435c7a4cd8e0ee39ba45f21ed37ad908a2a010784ccef25', 'f28905787faf43b6d3e0aeb100f149c82f44231bd119180c182d346545758256',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier02.3', 'AthenaToken_BRS12_HeroSelection_02', 'VTID_453_DesertOpsCamo_Tattoo_StyleD', 1, 'Cosmetics.Variant.Channel.Emissive', 'Cosmetics.Variant.Property.Emissive4', False,
        None, None, None, None,
        '3056dde04634709eb34226e0d267766bd262d2f755b389a4fe26bb9a9b9f9bb4', 'e0d01cacb232e43b9d2206490023253be2c5e99a320db7fc7e913e2830bc6f10',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier02.4', 'AthenaToken_BRS12_HeroSelection_02', 'VTID_454_DesertOpsCamo_Tattoo_StyleE', 1, 'Cosmetics.Variant.Channel.Emissive', 'Cosmetics.Variant.Property.Emissive5', False,
        None, None, None, None,
        '0caf149e3cc8e68c7cfffaef0633c258c1c7526a532953ad719694d0ab1e4320', '6a670b7b093cc5d584a1923ec643d28502e6dc670a77b896193ecdb4c8eff6e8',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier03.0', 'AthenaToken_BRS12_HeroSelection_03', 'VTID_531_DesertOpsCamo_Sleeves_StyleA', 1, 'Cosmetics.Variant.Channel.ClothingColor', 'Cosmetics.Variant.Property.Color.003', True,
        False, None, None, True,
        '63e7305e56e2497d06465fddb5eae8313c539609e0aaf53a7504f20986858c8a', 'ae7b17b37845c875fa71b33376c5ddb4faaf1f01b2f24d656622d5d3add016bc',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier03.1', 'AthenaToken_BRS12_HeroSelection_03', 'VTID_455_DesertOpsCamo_Sleeves_StyleB', 1, 'Cosmetics.Variant.Channel.ClothingColor', 'Cosmetics.Variant.Property.Color.004', False,
        None, None, None, None,
        '8218c5d0b6da2b9e8e6fb436782dcebacc573c4473bf61224277f46d6f8a9baa', '49e19684f0742bec4f4ae3a3fb5abe46f36e66a86993b32ea170587ad7e69261',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier03.2', 'AthenaToken_BRS12_HeroSelection_03', 'VTID_456_DesertOpsCamo_Sleeves_StyleC', 1, 'Cosmetics.Variant.Channel.ClothingColor', 'Cosmetics.Variant.Property.Color.005', False,
        None, None, None, None,
        '7ddc587f61e4fa66cfc99ccd7dba18e8332c71b5f387ad4419a3b3827c8f1526', '7ee4840f56f097c36d438f3661e52c09c15594d3ee8f6de182ea3bc629178925',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier04.0', 'AthenaToken_BRS12_HeroSelection_04', 'VTID_533_DesertOpsCamo_Boot_StyleA', 1, 'Cosmetics.Variant.Channel.Particle', 'Cosmetics.Variant.Property.Particle1', True,
        False, None, None, True,
        'e4acfa108ea2c07e2ef3fd3c2d3f718fc6ca7d5beb3223b692a737fd062188d8', '659d007846be411464845901688317aa326e326d5f4457171aa9f1cf5463fc90',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier04.1', 'AthenaToken_BRS12_HeroSelection_04', 'VTID_465_DesertOpsCamo_Boot_StyleB', 1, 'Cosmetics.Variant.Channel.Particle', 'Cosmetics.Variant.Property.Particle2', False,
        None, None, None, None,
        'c362519b9e010aae11ba7049482660c37bd2b580135de7a63be1ba728afd8f95', 'bf7647b6ae0a107c577b613ab94a99fafbfc3ff14d944a5872a533fd3462d85d',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier04.2', 'AthenaToken_BRS12_HeroSelection_04', 'VTID_466_DesertOpsCamo_Boot_StyleC', 1, 'Cosmetics.Variant.Channel.Particle', 'Cosmetics.Variant.Property.Particle3', False,
        None, None, None, None,
        '8204ce62d4a21c972bc8bcbb3b7eefecc19d5716583056a69ad26bdf62b20963', '112805d4c7e32380728a23da0431c81e5c4a77cc5b58457901775d8b78e10905',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier04.3', 'AthenaToken_BRS12_HeroSelection_04', 'VTID_467_DesertOpsCamo_Boot_StyleD', 1, 'Cosmetics.Variant.Channel.Particle', 'Cosmetics.Variant.Property.Particle4', False,
        None, None, None, None,
        '764b6b314610fad3425137de938c3b2fb07faf2cd5444fa0d47b1cf6434efa20', 'ebbe6f097bbadd373949727fa2f2cd0cf5293c9d9b74bf4c299585d40816d3d5',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier05.0', 'AthenaToken_BRS12_HeroSelection_05', 'VTID_534_DesertOpsCamo_ShirtColor_StyleA', 1, 'Cosmetics.Variant.Channel.Material', 'Cosmetics.Variant.Property.Mat1', True,
        False, None, None, True,
        'efe301c5de8f6d27ef0bb1b333a410d788fd85243bc2d198c1f17106c405abf0', '2070495ce36a5d4be26d45b184833c85ff8e82f4247b04df56d02c6fb92ace99',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier05.1', 'AthenaToken_BRS12_HeroSelection_05', 'VTID_458_DesertOpsCamo_ShirtColor_StyleB', 1, 'Cosmetics.Variant.Channel.Material', 'Cosmetics.Variant.Property.Mat2', False,
        None, None, None, None,
        '14b74b7100e4013c83d18c6a29a9f2bb4d7847e7a9e4d47930b30369a89dd131', '46b6a8b224af87a4796fb89c700d9be189957c965d6876d3a68f4815cc465f34',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier05.2', 'AthenaToken_BRS12_HeroSelection_05', 'VTID_459_DesertOpsCamo_ShirtColor_StyleC', 1, 'Cosmetics.Variant.Channel.Material', 'Cosmetics.Variant.Property.Mat3', False,
        None, None, None, None,
        'e2e83da2a20b0bdfa341717eda4a45bc88fd443c2b655f070b02db08bbc564ab', 'b60c3b735a5a05006c169822f79415747d3e8361a3780ca5b481749d9fb6cda4',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier05.3', 'AthenaToken_BRS12_HeroSelection_05', 'VTID_460_DesertOpsCamo_ShirtColor_StyleD', 1, 'Cosmetics.Variant.Channel.Material', 'Cosmetics.Variant.Property.Mat4', False,
        None, None, None, None,
        'c3824f3585a17bba2f249bed5dfd59c911d752ceea6a80a9269552491e8a4c24', '9a19dfc0e846851e6cdfa64cd8f9e7f88c1c73118a1b6cde1fdafbcba5d408a4',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier05.4', 'AthenaToken_BRS12_HeroSelection_05', 'VTID_461_DesertOpsCamo_ShirtColor_StyleE', 1, 'Cosmetics.Variant.Channel.Material', 'Cosmetics.Variant.Property.Mat5', False,
        None, None, None, None,
        '0924d1eb89b31ef64a51f018d5a063de86b1a43ab0df72b8f898b729f9c3bacc', '872d92741401a67c1b9660a2026e165c504e13f981cc6cfc431ffce0d41b128f',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier05.5', 'AthenaToken_BRS12_HeroSelection_05', 'VTID_462_DesertOpsCamo_ShirtColor_StyleF', 1, 'Cosmetics.Variant.Channel.Material', 'Cosmetics.Variant.Property.Mat6', False,
        None, None, None, None,
        '7cf5d4f04c8deb7c39518720c489d17b0d9ef5cb59646d9a71626b7fea8498d8', '5aea135f59ecb40f3d4fd473a1a61f4c5c153926e9c3cac195c1326dab518ffb',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier05.6', 'AthenaToken_BRS12_HeroSelection_05', 'VTID_463_DesertOpsCamo_ShirtColor_StyleG', 1, 'Cosmetics.Variant.Channel.Material', 'Cosmetics.Variant.Property.Mat7', False,
        None, None, None, None,
        'c1e2f518b16edfd38914932cf1bc28d2a39a0a0d064ae90abd68289ac3d0974e', '2d5c5217a333ca85a0d624320c1166fc611db2f474d7407ff688ed54585fd296',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier05.7', 'AthenaToken_BRS12_HeroSelection_05', 'VTID_464_DesertOpsCamo_ShirtColor_StyleH', 1, 'Cosmetics.Variant.Channel.Material', 'Cosmetics.Variant.Property.Mat8', False,
        None, None, None, None,
        '4cdf5a826b1c2df0160ab6ca168ef1701354698d21758ce87624ca0390638b5d', '86cc27b452bc692a10af845d3783fc09ec62440d230f4af65035cc6c106329bc',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier05.8', 'AthenaToken_BRS12_HeroSelection_05', 'VTID_488_DesertOpsCamo_ShirtColor_StyleI', 1, 'Cosmetics.Variant.Channel.Material', 'Cosmetics.Variant.Property.Mat17', False,
        None, None, None, None,
        '33b26caefb951c813b6f7b0bdbdb2007db43dc301735f8bf00d0c3c21b1970cb', '13e9e4546d700f195dd168faf27b400d66219d9e2a1d5a2f64736fd7ad1a0fb3',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier05.9', 'AthenaToken_BRS12_HeroSelection_05', 'VTID_489_DesertOpsCamo_ShirtColor_StyleJ', 1, 'Cosmetics.Variant.Channel.Material', 'Cosmetics.Variant.Property.Mat18', False,
        None, None, None, None,
        'e504f1754bd97e5c3cb73a820cf317d5d7143b8d9db95551ef1e1534c458f523', '9f353eff4ee611168dcd00bba12a21517e9dc7cb433b5dce0a3ff98b23e8c052',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier05.10', 'AthenaToken_BRS12_HeroSelection_05', 'VTID_490_DesertOpsCamo_ShirtColor_StyleK', 1, 'Cosmetics.Variant.Channel.Material', 'Cosmetics.Variant.Property.Mat19', False,
        None, None, None, None,
        '5dd482c3865cc215495741fcb3453675cbaab323063dfe9abd23a76e1bc5b29c', '3f8f9cb4d964ad0afc347ebdf289267b3480df6a88bcb407070761d434238387',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier05.11', 'AthenaToken_BRS12_HeroSelection_05', 'VTID_524_DesertOpsCamo_ShirtColor_StyleL', 1, 'Cosmetics.Variant.Channel.Material', 'Cosmetics.Variant.Property.Mat20', False,
        None, None, None, None,
        'c96100ad5615e4dd8f8845405d911d9f0eecd43fc5545da17bed5a840e45f679', '83e00897d0710538a3e50c8de4a446196e7c42c7df541dc22042c7c361379508',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier05.12', 'AthenaToken_BRS12_HeroSelection_05', 'VTID_525_DesertOpsCamo_ShirtColor_StyleM', 1, 'Cosmetics.Variant.Channel.Material', 'Cosmetics.Variant.Property.Mat21', False,
        None, None, None, None,
        '57de7cdc959eb504dfc82332be6aa49f126eaf6bebeef4a506add17ccfc74024', '84003772645bd0235a97ab5918c80019d11c1fc0819ccd4c3faa9fbe17111cf0',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier05.13', 'AthenaToken_BRS12_HeroSelection_05', 'VTID_526_DesertOpsCamo_ShirtColor_StyleN', 1, 'Cosmetics.Variant.Channel.Material', 'Cosmetics.Variant.Property.Mat22', False,
        None, None, None, None,
        '2ce982602f544cbcef1a8fe07b29372b77f7941a1c09b058e78f282ee992d7e5', '55bcbf19ad828f5e3a0f3427559a333e5e6f76f1d55f3d195ca3012e2cb4351a',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier05.14', 'AthenaToken_BRS12_HeroSelection_05', 'VTID_527_DesertOpsCamo_ShirtColor_StyleO', 1, 'Cosmetics.Variant.Channel.Material', 'Cosmetics.Variant.Property.Mat23', False,
        None, None, None, None,
        '4dbe37ff8a794d0adc68f43b7a4786f75595c814b36be5a1b4bc5a892e3213e9', 'fcaf22fce0f4633f5062cd21ec13a3baf5d30e92c9eb461a60dc2c3bfe7df19c',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier05.15', 'AthenaToken_BRS12_HeroSelection_05', 'VTID_528_DesertOpsCamo_ShirtColor_StyleP', 1, 'Cosmetics.Variant.Channel.Material', 'Cosmetics.Variant.Property.Mat24', False,
        None, None, None, None,
        '03d20f49fb126305369db54d654faf9979b4b026e960be141361185835d05407', 'fdcfed1bfc0294e61b19249c70f12a1f3ed1cf5da9be7aac60bf868c104e7f7e',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier06.0', 'AthenaToken_BRS12_HeroSelection_06', 'VTID_538_DesertOpsCamo_Vest_StyleA', 1, 'Cosmetics.Variant.Channel.Progressive', 'Cosmetics.Variant.Property.Stage1', True,
        False, None, None, True,
        'c6ec2e0dfa92fc2cd3ad5591a6b25e9140fc4e23c752d1309cc6c7fa9d6b0401', '06c2300575b65bb8ed0ab94d1bd0b2411c72c0b31f8f7888f11b4d762e90cbbc',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier06.1', 'AthenaToken_BRS12_HeroSelection_06', 'VTID_483_DesertOpsCamo_Vest_StyleB', 1, 'Cosmetics.Variant.Channel.Progressive', 'Cosmetics.Variant.Property.Stage2', False,
        None, None, None, None,
        '296cba7e63a38c1d0feca5bc95e28bd14ae9228b6b8f90e026551f1f854ea5a7', '36211848242637a8b07a82c5e1d9846c82c7fbeebdd4979460fa7f53bdf27b96',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier06.2', 'AthenaToken_BRS12_HeroSelection_06', 'VTID_484_DesertOpsCamo_Vest_StyleC', 1, 'Cosmetics.Variant.Channel.Progressive', 'Cosmetics.Variant.Property.Stage3', False,
        None, None, None, None,
        '6ee1ce6af34e092fee4a3d6b7bda073b9f8f4def299409b6b2ad50d9d621dc08', 'a206557d67fdfdcd704cfd0174ec8bb3de16b1755cbb33667361e6891948e4cd',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier06.3', 'AthenaToken_BRS12_HeroSelection_06', 'VTID_485_DesertOpsCamo_Vest_StyleD', 1, 'Cosmetics.Variant.Channel.Progressive', 'Cosmetics.Variant.Property.Stage4', False,
        None, None, None, None,
        'c4b240db4275df3c81f80c3de39f2f48f6721c0356c35c50565b45a0dbc5b2c5', '35ca42a41d25c5aafac063d8d45e4af549a2f6c0de3eb1d93c2e385798eed274',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier06.4', 'AthenaToken_BRS12_HeroSelection_06', 'VTID_486_DesertOpsCamo_Vest_StyleE', 1, 'Cosmetics.Variant.Channel.Progressive', 'Cosmetics.Variant.Property.Stage5', False,
        None, None, None, None,
        '590d7f853190daf8ae35fe67c691c350af88b508cba950b5ef389deef2a58625', '8a4557b9be9c0769e26a54732474e7a7c3354a048fe47797e95eb40e630efa2f',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier07.0', 'AthenaToken_BRS12_HeroSelection_07', 'VTID_535_DesertOpsCamo_Helmet_StyleA', 1, 'Cosmetics.Variant.Channel.Parts', 'Cosmetics.Variant.Property.Stage6', True,
        False, None, None, True,
        '1f61e009c6612b0ed13177fe674f5633cea4377b94d29b007dc2c9fa75fc1024', '94d64e7a03571cccace62591397dc92cf6bcc04dbd3c9e49f841c27a1b7619d0',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier07.1', 'AthenaToken_BRS12_HeroSelection_07', 'VTID_468_DesertOpsCamo_Helmet_StyleB', 1, 'Cosmetics.Variant.Channel.Parts', 'Cosmetics.Variant.Property.Stage7', False,
        None, None, None, None,
        'a6b30a61b5c858a4e2189f958b77c45b94410b237ccfd1e3b0f354b7aab6f0e7', '881d3830fe477a6ad43332d550357c29d999679cfa8e5536066e200e430251ce',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier07.2', 'AthenaToken_BRS12_HeroSelection_07', 'VTID_469_DesertOpsCamo_Helmet_StyleC', 1, 'Cosmetics.Variant.Channel.Parts', 'Cosmetics.Variant.Property.Stage8', False,
        None, None, None, None,
        '37d021240dfb8206605ce7298bd3706051fc3071d1876df6022b89f134d2de2a', '9b72a641cc8c9a7e6f2e5ad56935943bdc0fadf1b7a54ed4cd0bcbb0a5b22c51',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier07.3', 'AthenaToken_BRS12_HeroSelection_07', 'VTID_470_DesertOpsCamo_Helmet_StyleD', 1, 'Cosmetics.Variant.Channel.Parts', 'Cosmetics.Variant.Property.Stage9', False,
        None, None, None, None,
        '122c5381144caec4b73eda81ae2be70cb02cd3feba7cdb1e800cdf59cf5bd46c', 'b4091dac56a42d10461f06836cf85c19b884d14db852481674bdf8771c00cdb1',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier07.4', 'AthenaToken_BRS12_HeroSelection_07', 'VTID_471_DesertOpsCamo_Helmet_StyleE', 1, 'Cosmetics.Variant.Channel.Parts', 'Cosmetics.Variant.Property.Stage10', False,
        None, None, None, None,
        'f0eb7a967cb902360d4c3e3d506b28e5ebf0a9303c4d37db6d80aaeb07ea9832', '3c58e50d5949448af2bc8a856ab9a25433295a8460135ad5c85964d994829753',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier07.5', 'AthenaToken_BRS12_HeroSelection_07', 'VTID_568_DesertOpsCamo_Helmet_StyleF', 1, 'Cosmetics.Variant.Channel.Parts', 'Cosmetics.Variant.Property.Stage11', False,
        None, None, None, None,
        '4ca5124af61974f0c995cc13f877465aab727e84f7b67f2d972d56bb684d0c8c', 'd94720d4d26da4560df4ba1664b6cf9f5ac71bd547769c0c0bcb90403494235e',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier08.0', 'AthenaToken_BRS12_HeroSelection_08', 'VTID_536_DesertOpsCamo_PantsColor_StyleA', 1, 'Cosmetics.Variant.Channel.JerseyColor', 'Cosmetics.Variant.Property.Mat9', True,
        False, None, None, True,
        '28ee11e41c440667be19f01a77de6450781a4371f6bb8c7aad6dd2e672cad4bc', '8e89c0e2f5f1495884fe3fd8df17f45d41bce20773204aacb421550449406531',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier08.1', 'AthenaToken_BRS12_HeroSelection_08', 'VTID_472_DesertOpsCamo_PantsColor_StyleB', 1, 'Cosmetics.Variant.Channel.JerseyColor', 'Cosmetics.Variant.Property.Mat10', False,
        None, None, None, None,
        'ba03f46f299002775dcb525778eb9b2af940c20e3556e47b107cd9c28523b037', '0cb9955fd4818393c705bba24b4fbcc7e8083832eee966532258801e181e5189',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier08.2', 'AthenaToken_BRS12_HeroSelection_08', 'VTID_473_DesertOpsCamo_PantsColor_StyleC', 1, 'Cosmetics.Variant.Channel.JerseyColor', 'Cosmetics.Variant.Property.Mat11', False,
        None, None, None, None,
        'c216bcb8f2c040158f86de790378134c03d7fefdb8727540093bf34937650f91', '9bf7cddb8c80f5ac7e62d5f1659791450bd836c6166b4de5ea18fb5ff4553888',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier08.3', 'AthenaToken_BRS12_HeroSelection_08', 'VTID_474_DesertOpsCamo_PantsColor_StyleD', 1, 'Cosmetics.Variant.Channel.JerseyColor', 'Cosmetics.Variant.Property.Mat12', False,
        None, None, None, None,
        '8523591611e8a5acd7d9352a754be606aa811caedf48a600d9387318ba43f637', '440edddff3d85e7590e91c9a190c2ba62bb76063c5d29025dab0c3a9a43bcd4c',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier08.4', 'AthenaToken_BRS12_HeroSelection_08', 'VTID_475_DesertOpsCamo_PantsColor_StyleE', 1, 'Cosmetics.Variant.Channel.JerseyColor', 'Cosmetics.Variant.Property.Mat13', False,
        None, None, None, None,
        '7500f0b03ebbc964cbefced63f714d7a37b61f4aac8f8459d1b29ff883059415', '461cc9a51faaea935e5f729b1b1aeb90879c291d7959c93cd30f5e563d895ce9',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier08.5', 'AthenaToken_BRS12_HeroSelection_08', 'VTID_476_DesertOpsCamo_PantsColor_StyleF', 1, 'Cosmetics.Variant.Channel.JerseyColor', 'Cosmetics.Variant.Property.Mat14', False,
        None, None, None, None,
        '401c3ae4f901bd6878262c3b3a31728c81e255affadf37215de6faaf8d14f2b1', '05371184e0769a7b1860d831e946400318ccdccf5ef262e4be5bacbb2be52c8a',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier08.6', 'AthenaToken_BRS12_HeroSelection_08', 'VTID_477_DesertOpsCamo_PantsColor_StyleG', 1, 'Cosmetics.Variant.Channel.JerseyColor', 'Cosmetics.Variant.Property.Mat15', False,
        None, None, None, None,
        'b14c828c4fb9a436d4768d22a25eb958fbc629ea5747ba640b766b0c5f24864e', '9faf670e49124540c794e63d03d6b9bab69db64650df9cc22ddd8a9d465aa491',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier08.7', 'AthenaToken_BRS12_HeroSelection_08', 'VTID_478_DesertOpsCamo_PantsColor_StyleH', 1, 'Cosmetics.Variant.Channel.JerseyColor', 'Cosmetics.Variant.Property.Mat16', False,
        None, None, None, None,
        '3472b7fe88186bde046e2545855d5fa01f8d1d9697adb2e3353bd798d04c64d4', 'd252e76b9579b9f76103d81a8bd17f0cda39f35913fcad6c6d8b7ba6aaddbacc',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier09.0', 'AthenaToken_BRS12_HeroSelection_09', 'VTID_537_DesertOpsCamo_FacePaint_StyleA', 1, 'Cosmetics.Variant.Channel.Mesh', 'Cosmetics.Variant.Property.Color.009', True,
        False, None, None, True,
        '3fbbd6ce31e6abf0cb569a18ac08994460950f1e4216d1962309c4d30e7905d8', 'd2384e4183d1214d3d629a5eef16b955df368fa45cc6dcee91f03e573ab598ab',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier09.1', 'AthenaToken_BRS12_HeroSelection_09', 'VTID_479_DesertOpsCamo_FacePaint_StyleB', 1, 'Cosmetics.Variant.Channel.Mesh', 'Cosmetics.Variant.Property.Color.010', False,
        None, None, None, None,
        'f9cd5b24f329290bd3dab3e61f3e24a18775211f5a44bfd5b06c03220c557a71', 'd197cad0e1a01475034be25728f151685e2b397f37fb7701b264d8ba2ad65f63',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier09.2', 'AthenaToken_BRS12_HeroSelection_09', 'VTID_480_DesertOpsCamo_FacePaint_StyleC', 1, 'Cosmetics.Variant.Channel.Mesh', 'Cosmetics.Variant.Property.Color.011', False,
        None, None, None, None,
        '4d1c22561b6772af38a6a24939524dd6786414ef3716cd20760c85ff7249a40b', '7b7182116110122ca1a8df04a698e0c1ff9e09fbf804a19415125ed4909b1ad5',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier09.3', 'AthenaToken_BRS12_HeroSelection_09', 'VTID_481_DesertOpsCamo_FacePaint_StyleD', 1, 'Cosmetics.Variant.Channel.Mesh', 'Cosmetics.Variant.Property.Color.012', False,
        None, None, None, None,
        'afbd401e0223f6c5dbd2663a88bda3bb57f4c932a804a75ce3372a2f477072af', 'd796fe8684a92bbe407c193f466a2363460fc83fed7ae750244f1259246a840b',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier09.4', 'AthenaToken_BRS12_HeroSelection_09', 'VTID_482_DesertOpsCamo_FacePaint_StyleE', 1, 'Cosmetics.Variant.Channel.Mesh', 'Cosmetics.Variant.Property.Color.013', False,
        None, None, None, None,
        '83f18903b4712f6c93198ea947780f686c7c776b55d5c7a4d60df0f6a721428b', 'd881f4f2b0c90812d51e54acc64ddc2389fc94563c61e03812d71fe2a6725e85',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier10.0', 'AthenaToken_BRS12_HeroSelection_10', 'VTID_532_DesertOpsCamo_Scarf_StyleA', 1, 'Cosmetics.Variant.Channel.Pattern', 'Cosmetics.Variant.Property.Color.001', True,
        False, None, None, True,
        '48f5fbe6b2ecc46917201fe013246d61a254d067cac80ca445b70a94e5a98a34', '19876dcd967bd6949892eb7c7820d2e8f50e2736fc005c0749263d12d6a91ba6',
        1, 1, 0, True, '', ''),
    Node(
        'ERG.Node.Tier10.1', 'AthenaToken_BRS12_HeroSelection_10', 'VTID_457_DesertOpsCamo_Scarf_StyleB', 1, 'Cosmetics.Variant.Channel.Pattern', 'Cosmetics.Variant.Property.Color.002', False,
        None, None, None, None,
        '58fed14ca178875bd983ad4cd56e338c13e474f5f33cf50abe9edfbee5ea38e4', '0639a99e3df23495c167a37b00e7104d186ed3b738691edcadfecd095d6476a2',
        1, 1, 0, True, 'RichColor', 'CID_695_Athena_Commando_F_DesertOpsCamo'),
)
NODES = {node.node_id: node for node in _NODES}
NODES_BY_REWARD_TOKEN = {node.reward_token: node for node in _NODES}

# Native 0x1426ffaf0 formats four raw IEEE754 RGBA words. The tagged LinearColor
# payload in this target's hashed uexp at 76168 is sixteen zero bytes; the
# inspector's stringified "000000" alone did not establish that numeric value.
RICH_COLOR = {
    "export": "FortCosmeticRichColorVariant_0",
    "property_tag": "Cosmetics.Variant.Property.RichColor",
    "config_channel": "Cosmetics.Variant.Channel.RichColor",
    "start_unlocked": False,
    "color_parameter": "Cloth Fuzz Tint",
    "default_starting_color": (0.0, 0.0, 0.0, 0.0),
    "default_raw_words": "00000000000000000000000000000000",
}
UNRESOLVED = (
    "Original backend response and validation policies",
    "Purchase-token processing, ownership, tier enforcement and atomicity",
    "SetRewardGraphConfig mutations and accepted state values",
)
