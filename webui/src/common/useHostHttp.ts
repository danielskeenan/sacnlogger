import {useMemo} from "react";
import axios from 'axios';
import {SERVER_ORIGIN} from "./constants.ts";
import {ByteBuffer} from "flatbuffers";

/**
 * Communicate with the host using HTTP requests.
 */
export default function useHostHttp() {
    return useMemo(() => {
        const axiosInstance = axios.create({
            baseURL: SERVER_ORIGIN,
            allowAbsoluteUrls: false,
            timeout: 30000,
            headers: {
                Accept: "application/octet-stream",
            },
            responseType: "arraybuffer",
            transformResponse: [
                function (data: ArrayBuffer) {
                    const bytes = new Uint8Array(data);
                    return new ByteBuffer(bytes);
                },
            ],
        });

        return {
            axiosInstance,
            hostGet: axiosInstance.get<ByteBuffer>,
        };
    }, []);
}
