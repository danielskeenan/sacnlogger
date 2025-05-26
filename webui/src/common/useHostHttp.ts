import {useMemo} from "react";
import axios, {type AxiosRequestConfig, type AxiosResponse} from 'axios';
import {SERVER_ORIGIN} from "./constants.ts";
import {ByteBuffer} from "flatbuffers";

/**
 * Communicate with the host using HTTP requests.
 */
export default function useHostHttp<GetResponseType = ByteBuffer, PostRequestType = Uint8Array, PostResponseType = string>() {
    return useMemo(() => {
        const axiosInstance = axios.create({
            baseURL: SERVER_ORIGIN,
            allowAbsoluteUrls: false,
            timeout: 30000,
        });

        const hostGet = (url: string, config?: AxiosRequestConfig) => {
            const mergedConfig: AxiosRequestConfig = {
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
                ...config,
            };
            return axiosInstance.get<GetResponseType>(url, mergedConfig);
        }

        const hostPost = (url: string, data: PostRequestType, config?: AxiosRequestConfig) => {
            const mergedConfig: AxiosRequestConfig = {
                headers: {
                    "Accept": "text/plain",
                    "Content-Type": "application/octet-stream",
                },
                responseType: "text",
                ...config,
            };
            return axiosInstance.post<PostResponseType, AxiosResponse<PostResponseType>, PostRequestType>(url, data, mergedConfig);
        };

        return {
            axiosInstance,
            hostGet: hostGet,
            hostPost: hostPost,
        };
    }, []);
}
