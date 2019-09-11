import highlight, { getLanguage } from 'highlight.js';
import 'highlight.js/styles/atom-one-light.css';
import React, { PureComponent } from 'react';

highlight.registerLanguage('c', () => getLanguage('cpp'));
highlight.registerLanguage('cpp', () => getLanguage('cpp'));
highlight.registerLanguage('json', () => getLanguage('json'));

interface Props {
    language: string;
}

export class CodeHighlight extends PureComponent<Props> {
    state = { loaded: false };
    codeNode = React.createRef<HTMLElement>();

    componentDidMount() {
        const { language } = this.props;
        if (language) {
            this.setState(
                () => {
                    return { loaded: true };
                },
                () => {
                    this.highlight();
                }
            );
        } else {
            this.setState({ loaded: true });
        }
    }

    componentDidUpdate() {
        this.highlight();
    }

    highlight() {
        this.codeNode && this.codeNode.current && highlight.highlightBlock(this.codeNode.current);
    }

    render() {
        const { language, children } = this.props;
        const { loaded } = this.state;
        if (!loaded) return '';

        return <pre>
            <code ref={this.codeNode} className={language}>{children}</code>
        </pre>;
    }
}
