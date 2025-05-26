import {type RefObject, useCallback, useRef} from "react";

type SetFocusCallback = (options?: FocusOptions) => void;

/**
 * Allow programmatically focusing on an element.
 */
export default function useFocus<ElementType extends HTMLInputElement>(): [RefObject<ElementType | null>, SetFocusCallback] {
    const elementRef = useRef<ElementType>(null);
    const setFocus = useCallback((options?: FocusOptions) => {
        elementRef.current && elementRef.current.focus(options);
    }, [elementRef]);

    return [elementRef, setFocus];
}
