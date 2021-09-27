const { DeviceProvider } = require("./services/fr_deviceprovider.mjs");
const Stream = require('stream-browserify');

const OSC_MAX_CHANNELS = 20;

export default class Oscilloscope {
    constructor() {
        this.id = 0
        this.deviceId = 0
        this.name = ''
        this.desc = ''
        this.resolution_ns = 0;
        this.lastError = 0;
        this.channelStreams = []
        this.trig_time = new Date().getTime();
        this.channels = []

        this.deviceProvider = new DeviceProvider();

        // Переменные для измерения производительности
        this._byteCount = 0;
        this._msgCount = 0;
        this._timeLabel = new Date().getTime();
    }

    async openDataStream() {

        this.channels.forEach(channel => {
            channel.stream = new Stream.Readable({
                highWaterMark: 1, //STREAM_BUFFER_OBJECTS,
                objectMode: true,
                read() { }
            });
        });

        this._byteCount = 0;
        this._msgCount = 0;
        this._timeLabel = new Date().getTime();
        console.time(`The stream elapsed time(${this._timeLabel})`);

        try {
            let openStream = true;
            let oscHeader = await this.deviceProvider.reqOsc(this.id);
            this.trig_time = oscHeader.trig_time

            await this.deviceProvider.reqDataStream(this.deviceId, this.id, openStream);
        } catch (error) {
            this.lastError = error;
            console.error(error);
        }
    }

    async closeDataStream() {
        try {
            let openStream = false;
            await this.deviceProvider.reqDataStream(this.deviceId, this.id, openStream);
        } catch (error) {
            this.lastError = error;
            console.error(error);
        }

        for (var i = 1; i <= OSC_MAX_CHANNELS; ++i) {
            this.channels[i].stream.push(null);
            this.channels[i].stream.destroy;
            this.channels[i].stream = null;
        }
    }

    stream(socketData) {
        for (var i = 1; i <= OSC_MAX_CHANNELS; ++i) {
            let data = this.parse(socketData, i);
            if (data == -1) {
                continue;
            }
            if (this.channels[i].stream !== null) {
                this.channels[i].stream.push(data);
            }
        }
    }

    parse(socketData, ch) {

        if (socketData.values == undefined) {
            return -1;
        }

        const isFinished = (socketData.error === 2);
        if (isFinished) {
            if (ch == OSC_MAX_CHANNELS) {
                console.log(`The osc stream is finished, device id = ${this.deviceId}, received items = ${this._msgCount}, bytes = ${this._byteCount}`);
                console.timeEnd(`The stream elapsed time(${this._timeLabel})`);
            }
            return null;
        }

        let chValues = socketData.values[ch];
        if (chValues === undefined || chValues.length == 0) {
            return null;
        }

        let receivedBytes = JSON.stringify(chValues[0]).length * chValues.length;
        this._byteCount += receivedBytes;
        // console.log('Received bytes = ' + receivedBytes);

        this._msgCount++;

        let time_ns = socketData.time - chValues.length * this.resolution_ns;
        let trig_time = this.trig_time;
        let values = chValues.map(val => {
            let time = time_ns * 0.001 - trig_time;
            time_ns += this.resolution_ns;

            return { val, time };
        })
        return values;
    }
}