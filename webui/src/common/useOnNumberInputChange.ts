import {type ChangeEvent, useCallback} from "react";

type OnNumberChangeValidator = ((val: number) => boolean) | undefined;

/**
 * Create callback to handle an onChange() event from an HTML number input.
 * @param setter Called with the new Number value.
 * @param validator A validator that returns true if the value is allowed to be set.
 */
export default function useOnNumberChange(setter: (val: number) => void, validator: OnNumberChangeValidator) {
    return useCallback((e: ChangeEvent<HTMLInputElement>) => {
        const val = Number(e.target.value);

        // Is this really a number?
        if (!Number.isSafeInteger(val)) {
            return;
        }

        // Is it valid? (optional)
        if (validator && !validator(val)) {
            return;
        }

        // All good, set value.
        setter(val);
    }, [setter, validator]);
}
