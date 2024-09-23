from .Exceptions import DfmEntryParserException
from .EntryHeader import EntryHeader, EntryType
from .MqttTopic import generate_mqtt_topic
from .BinaryGenerator import BinaryGenerator, DaToolsPayload


class DfmEntryParser:

    @staticmethod
    def parse(raw_entry_header: bytes) -> bytes:
        entry_header = EntryHeader(raw_entry_header)

        if not entry_header.is_valid:
            raise DfmEntryParserException("Invalid entry header provided")

        mqtt_topic = generate_mqtt_topic(entry_header)
        payload = BinaryGenerator.generate_output(mqtt_topic, entry_header.data)

        return payload

    def get_entry_data(raw_entry_header: bytes) -> bytes:
        entry_header = EntryHeader(raw_entry_header)

        if not entry_header.is_valid:
            raise DfmEntryParserException("Invalid entry header provided")
        
        # print("DeviceID: " + entry_header.device_name);
        # print("SessionID: " + entry_header.session_id);
        
        return entry_header.data
        
    def get_entry_data_modified(raw_entry_header: bytes, session_id: str, device_id: str) -> bytes:
        entry_header = EntryHeader(raw_entry_header)

        if not entry_header.is_valid:
            raise DfmEntryParserException("Invalid entry header provided")

        if (entry_header.session_id == '$DUMMY_SESSION_ID'):
            entry_header.set_session_id(session_id)
        
        if (entry_header.device_name == '$DUMMY_DEVICE_ID'):
            entry_header.set_device_name(device_id)
        
        topic = generate_mqtt_topic(entry_header)
            
        return entry_header.data, topic

    def is_header(raw_entry_header: bytes) -> bool:
        entry_header = EntryHeader(raw_entry_header)
        
        if entry_header.entry_type == EntryType.alert:
            return True
        else:
            return False

    @staticmethod
    def get_topic(raw_entry_header: bytes) -> bytes:
        entry_header = EntryHeader(raw_entry_header)

        if not entry_header.is_valid:
            raise DfmEntryParserException("Invalid entry header provided")

        topic = generate_mqtt_topic(entry_header)

        return topic
