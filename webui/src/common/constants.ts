import {localStorageGet, LocalStorageItem} from "./localStorage.ts";

export const APP_NAME = "sACN Logger";

// Allow overriding the backend URL during development so the frontend can run from Vite's server.
export const SERVER_ORIGIN = (() => {
    const serverOverride = localStorageGet<string | null>(LocalStorageItem.SERVER, null);
    return serverOverride ?? document.location.origin;
})();

export const UNIVERSE_MIN = 1;
export const UNIVERSE_MAX = 63999;

export function isValidUniverse(universe: number) {
    return Number.isSafeInteger(universe) && universe >= UNIVERSE_MIN && universe <= UNIVERSE_MAX;
}
